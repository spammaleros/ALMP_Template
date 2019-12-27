// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
	AkAudioBankGenerationHelpers.cpp: Wwise Helpers to generate banks from the editor and when cooking.
------------------------------------------------------------------------------------*/

#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioStyle.h"
#include "AkAuxBus.h"
#include "AkAudioBank.h"
#include "AkAudioEvent.h"
#include "AkUnrealHelper.h"
#include "AkSettingsPerUser.h"
#include "SGenerateSoundBanks.h"
#include "AkSettings.h"
#include "AssetRegistryModule.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/FileHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ProjectDescriptor.h"
#include "PlatformInfo.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/MonitoredProcess.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"
#include "Editor.h"
#include "Framework/Docking/TabManager.h"

#include "Platforms/AkUEPlatform.h"

#define LOCTEXT_NAMESPACE "AkAudio"

/** Whether we want the Cooking process to use Wwise to Re-generate banks.			*/
bool GIsWwiseCookingSoundBanks = true;

DEFINE_LOG_CATEGORY(LogAkBanks);

namespace WwiseBnkGenHelper
{
	FString GetWwiseApplicationPath()
	{
		const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
		FString ApplicationToRun;
		ApplicationToRun.Empty();

		if (AkSettingsPerUser)
		{
#if PLATFORM_WINDOWS
			ApplicationToRun = AkSettingsPerUser->WwiseWindowsInstallationPath.Path;
#else
			ApplicationToRun = AkSettingsPerUser->WwiseMacInstallationPath.FilePath;
#endif
			if (FPaths::IsRelative(ApplicationToRun))
			{
				ApplicationToRun = FPaths::ConvertRelativePathToFull(AkUnrealHelper::GetProjectDirectory(), ApplicationToRun);
			}
			if (!(ApplicationToRun.EndsWith(TEXT("/")) || ApplicationToRun.EndsWith(TEXT("\\"))))
			{
				ApplicationToRun += TEXT("/");
			}

#if PLATFORM_WINDOWS
			if (FPaths::FileExists(ApplicationToRun + TEXT("Authoring/x64/Release/bin/WwiseCLI.exe")))
			{
				ApplicationToRun += TEXT("Authoring/x64/Release/bin/WwiseCLI.exe");
			}
			else
			{
				ApplicationToRun += TEXT("Authoring/Win32/Release/bin/WwiseCLI.exe");
			}
			ApplicationToRun.ReplaceInline(TEXT("/"), TEXT("\\"));
#elif PLATFORM_MAC
			ApplicationToRun += TEXT("Contents/Tools/WwiseCLI.sh");
			ApplicationToRun = TEXT("\"") + ApplicationToRun + TEXT("\"");
#endif
		}

		return ApplicationToRun;
	}

	FString GetLinkedProjectPath()
	{
		// Get the Wwise Project Name from directory.
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		FString ProjectPath;
		ProjectPath.Empty();

		if (AkSettings)
		{
			ProjectPath = AkSettings->WwiseProjectPath.FilePath;

			ProjectPath = FPaths::ConvertRelativePathToFull(AkUnrealHelper::GetProjectDirectory(), ProjectPath);
#if PLATFORM_WINDOWS
			ProjectPath.ReplaceInline(TEXT("/"), TEXT("\\"));
#endif
		}

		return ProjectPath;
	}

	FString GetDefaultSBDefinitionFilePath()
	{
		FString TempFileName = AkUnrealHelper::GetProjectDirectory();
		TempFileName += TEXT("TempDefinitionFile.txt");
		return TempFileName;
	}

	/**
	 * Dump the bank definition to file
	 *
	 * @param in_DefinitionFileContent	Banks to include in file
	 */
	FString DumpBankContentString(TMap<FString, TSet<UAkAudioEvent*> >& in_DefinitionFileContent)
	{
		// This generate the Bank definition file.
		// 
		FString FileContent;
		if (in_DefinitionFileContent.Num())
		{
			for (TMap<FString, TSet<UAkAudioEvent*> >::TIterator It(in_DefinitionFileContent); It; ++It)
			{
				if (It.Value().Num())
				{
					FString BankName = It.Key();
					for (TSet<UAkAudioEvent*>::TIterator ItEvent(It.Value()); ItEvent; ++ItEvent)
					{
						FString EventName = (*ItEvent)->GetName();
						FileContent += BankName + "\t\"" + EventName + "\"\n";
					}
				}
			}
		}

		return FileContent;
	}

	FString DumpBankContentString(TMap<FString, TSet<UAkAuxBus*> >& in_DefinitionFileContent)
	{
		// This generate the Bank definition file.
		// 
		FString FileContent;
		if (in_DefinitionFileContent.Num())
		{
			for (TMap<FString, TSet<UAkAuxBus*> >::TIterator It(in_DefinitionFileContent); It; ++It)
			{
				if (It.Value().Num())
				{
					FString BankName = It.Key();
					for (TSet<UAkAuxBus*>::TIterator ItAuxBus(It.Value()); ItAuxBus; ++ItAuxBus)
					{
						FString AuxBusName = (*ItAuxBus)->GetName();
						FileContent += BankName + "\t-AuxBus\t\"" + AuxBusName + "\"\n";
					}
				}
			}
		}

		return FileContent;
	}

	bool GenerateDefinitionFile(TArray< TSharedPtr<FString> >& BanksToGenerate, TMap<FString, TSet<UAkAudioEvent*> >& BankToEventSet)
	{
		TMap<FString, TSet<UAkAuxBus*> > BankToAuxBusSet;
		{
			// Force load of event assets to make sure definition file is complete
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			TArray<FAssetData> EventAssets;
			AssetRegistryModule.Get().GetAssetsByClass(UAkAudioEvent::StaticClass()->GetFName(), EventAssets);

			for (int32 AssetIndex = 0; AssetIndex < EventAssets.Num(); ++AssetIndex)
			{
				FString EventAssetPath = EventAssets[AssetIndex].ObjectPath.ToString();
				UAkAudioEvent * pEvent = LoadObject<UAkAudioEvent>(NULL, *EventAssetPath, NULL, 0, NULL);
				if (BanksToGenerate.ContainsByPredicate([&](TSharedPtr<FString> Bank) {
					if (pEvent->RequiredBank)
					{
						return pEvent->RequiredBank->GetName() == *Bank;
					}
					return false;

				}))
				{
					if (pEvent->RequiredBank)
					{
						TSet<UAkAudioEvent*>& EventPtrSet = BankToEventSet.FindOrAdd(pEvent->RequiredBank->GetName());
						EventPtrSet.Add(pEvent);
					}
				}
			}

			// Force load of AuxBus assets to make sure definition file is complete
			TArray<FAssetData> AuxBusAssets;
			AssetRegistryModule.Get().GetAssetsByClass(UAkAuxBus::StaticClass()->GetFName(), AuxBusAssets);

			for (int32 AssetIndex = 0; AssetIndex < AuxBusAssets.Num(); ++AssetIndex)
			{
				FString AuxBusAssetPath = AuxBusAssets[AssetIndex].ObjectPath.ToString();
				UAkAuxBus * pAuxBus = LoadObject<UAkAuxBus>(NULL, *AuxBusAssetPath, NULL, 0, NULL);
				if (BanksToGenerate.ContainsByPredicate([&](TSharedPtr<FString> Bank) {
					if (pAuxBus->RequiredBank)
					{
						return pAuxBus->RequiredBank->GetName() == *Bank;
					}
					return false;
				}))
				{
					if (pAuxBus->RequiredBank)
					{
						TSet<UAkAuxBus*>& EventPtrSet = BankToAuxBusSet.FindOrAdd(pAuxBus->RequiredBank->GetName());
						EventPtrSet.Add(pAuxBus);
					}
				}
			}
		}

		FString DefFileContent = DumpBankContentString(BankToEventSet);
		DefFileContent += DumpBankContentString(BankToAuxBusSet);

		if (DefFileContent.IsEmpty())
		{
			UE_LOG(LogAkBanks, Error, TEXT("Your Wwise SoundBank definition file is empty, you should add a reference to a bank in your event assets."));
			return false;
		}

		return FFileHelper::SaveStringToFile(DefFileContent, *GetDefaultSBDefinitionFilePath());
	}

	FString GetBankGenerationFullDirectory(const TCHAR * in_szPlatformDir)
	{
		FString TargetDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(AkUnrealHelper::GetSoundBankDirectory(), in_szPlatformDir));

#if PLATFORM_WINDOWS
		TargetDir.ReplaceInline(TEXT("/"), TEXT("\\"));
#else
		TargetDir.ReplaceInline(TEXT("\\"), TEXT("/"));
#endif

		return TargetDir;
	}

	void GenerateCommandLineArguments(
		TArray< TSharedPtr<FString> >& in_rBankNames,
		TArray< TSharedPtr<FString> >& in_PlatformNames,
		FString& out_Command,
		FString& out_Params,
		const FString* WwisePathOverride = nullptr)
	{

#if PLATFORM_WINDOWS
		out_Command = GetWwiseApplicationPath();
		if (WwisePathOverride)
		{
			out_Command = *WwisePathOverride;
		}
#else
		out_Command = TEXT("/bin/sh");
		if (WwisePathOverride)
			out_Params += *WwisePathOverride;
		else
			out_Params += GetWwiseApplicationPath();
#endif


		//////////////////////////////////////////////////////////////////////////////////////
		// For more information about how to generate banks using the command line, 
		// look in the Wwise SDK documentation 
		// in the section "Generating Banks from the Command Line"
		//////////////////////////////////////////////////////////////////////////////////////

		// Put the project name within quotes " ".
		out_Params += FString::Printf(TEXT(" \"%s\""), *IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*GetLinkedProjectPath()));

		// add the flag to request to generate sound banks if required.
		out_Params += TEXT(" -GenerateSoundBanks");

		// For each bank to be generated, add " -Bank BankName"
		for (long i = 0; i < in_rBankNames.Num(); i++)
		{
			out_Params += FString::Printf(
				TEXT(" -Bank %s"),
				**in_rBankNames[i]
			);
		}

		// For each bank to be generated, add " -ImportDefinitionFile BankName"
		out_Params += FString::Printf(
			TEXT(" -ImportDefinitionFile \"%s\""),
			*IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*GetDefaultSBDefinitionFilePath())
		);

		// We could also specify the -Save flag.
		// It would cause the newly imported definition files to be persisted in the Wwise project files.
		// On the other hand, saving the project could cause the project currently being edited to be 
		// dirty if Wwise application is already running along with UnrealEditor, and the user would be 
		// prompted to either discard changes and reload the project, losing local changes.
		// You can uncomment the following line if you want the Wwise project to be saved in this process.
		// By default, we prefer to not save it.
		// 
		// out_Params += TEXT(" -Save");
		//

		// Generating for all asked platforms.
		if (in_PlatformNames.Num() == 0)
		{
			in_PlatformNames = AkUnrealPlatformHelper::GetAllSupportedWwisePlatforms();
		}

		for (int32 PlatformIdx = 0; PlatformIdx < in_PlatformNames.Num(); PlatformIdx++)
		{
			FString BankPath = GetBankGenerationFullDirectory(**in_PlatformNames[PlatformIdx]);
#if PLATFORM_MAC
			// Workaround: This parameter does not work with Unix-style paths. convert it to Windows style.
			BankPath = FString(TEXT("Z:")) + BankPath;
#endif

			out_Params += FString::Printf(
				TEXT(" -Platform %s -SoundBankPath %s \"%s\""),
				**in_PlatformNames[PlatformIdx],
				**in_PlatformNames[PlatformIdx],
				*BankPath
			);
		}

		// Here you can specify languages, if no language is specified, all languages from the Wwise project.
		// will be built.
		//#if PLATFORM_WINDOWS
		//		out_Params += TEXT(" -Language English(US)");
		//#else
		//		out_Params += TEXT(" -Language English\\(US\\)");
		//#endif

	}

	class FSoundBanksNotificationHandler
	{
	public:

		FSoundBanksNotificationHandler(TWeakPtr<SNotificationItem> InNotificationItemPtr, SNotificationItem::ECompletionState InCompletionState, const FText& InText, FOnBankGenerationComplete InOnSoundBankGenerationComplete, bool InExpireAndFadeout = true)
			: CompletionState(InCompletionState)
			, NotificationItemPtr(InNotificationItemPtr)
			, Text(InText)
			, bExpireAndFadeout(InExpireAndFadeout)
			, OnSoundBankGenerationComplete(InOnSoundBankGenerationComplete)

		{ }

		static void HandleDismissButtonClicked()
		{
			TSharedPtr<SNotificationItem> NotificationItem = ExpireNotificationItemPtr.Pin();
			if (NotificationItem.IsValid())
			{

				NotificationItem->SetExpireDuration(0.0f);
				NotificationItem->SetFadeOutDuration(0.0f);
				NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
				NotificationItem->ExpireAndFadeout();
				ExpireNotificationItemPtr.Reset();
			}
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			if (NotificationItemPtr.IsValid())
			{

				TSharedPtr<SNotificationItem> NotificationItem = NotificationItemPtr.Pin();
				NotificationItem->SetText(Text);

				if (bExpireAndFadeout)
				{
					ExpireNotificationItemPtr.Reset();
					NotificationItem->SetExpireDuration(3.0f);
					NotificationItem->SetFadeOutDuration(0.5f);
					NotificationItem->ExpireAndFadeout();
				}
				else
				{
					// Handling the notification expiration in callback
					ExpireNotificationItemPtr = NotificationItem;
				}
				NotificationItem->SetCompletionState(CompletionState);
				if (CompletionState == SNotificationItem::CS_Fail)
				{
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
				}
				else
				{
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
				}

				OnSoundBankGenerationComplete.ExecuteIfBound(CompletionState == SNotificationItem::CS_Success);
			}
		}

		static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
		ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FSoundBanksNotificationHandler, STATGROUP_TaskGraphTasks);
		}


	private:

		static TWeakPtr<SNotificationItem> ExpireNotificationItemPtr;

		SNotificationItem::ECompletionState CompletionState;
		TWeakPtr<SNotificationItem> NotificationItemPtr;
		FText Text;
		bool bExpireAndFadeout;
		FOnBankGenerationComplete OnSoundBankGenerationComplete;
	};
	TWeakPtr<SNotificationItem> FSoundBanksNotificationHandler::ExpireNotificationItemPtr;

	void HandleWwiseCliProcessCompleted(int32 ReturnCode, TWeakPtr<SNotificationItem> NotificationItemPtr, FOnBankGenerationComplete OnSoundBankGenerationComplete)
	{
		if (ReturnCode == 0 || ReturnCode == 2)
		{
			TGraphTask<FSoundBanksNotificationHandler>::CreateTask().ConstructAndDispatchWhenReady(
				NotificationItemPtr,
				SNotificationItem::CS_Success,
				LOCTEXT("SoundBankGenerationSucceeded", "SoundBank generation complete!"),
				OnSoundBankGenerationComplete
			);
		}
		else
		{
			TGraphTask<FSoundBanksNotificationHandler>::CreateTask().ConstructAndDispatchWhenReady(
				NotificationItemPtr,
				SNotificationItem::CS_Fail,
				LOCTEXT("SoundBankGenerationFailed", "SoundBank generation failed!"),
				OnSoundBankGenerationComplete
			);
		}
	}
	
	void HandleWwiseCliProcessCanceled(TWeakPtr<SNotificationItem> NotificationItemPtr, FOnBankGenerationComplete OnSoundBankGenerationComplete)
	{
		TGraphTask<FSoundBanksNotificationHandler>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Fail,
			LOCTEXT("SoundBankGenerationCanceled", "SoundBank generation canceled!"),
			OnSoundBankGenerationComplete
		);
	}

	static void ProcessWwiseCliOutput(FString Output, TWeakPtr<SNotificationItem> NotificationItemPtr)
	{
		if (!Output.IsEmpty() && !Output.Equals("\r"))
		{
			UE_LOG(LogAkBanks, Display, TEXT("%s"), *Output);
		}
	}

	int32 GenerateSoundBanksBlocking(TArray< TSharedPtr<FString> >& in_rBankNames, TArray< TSharedPtr<FString> >& in_PlatformNames, const FString* WwisePathOverride/* = nullptr*/)
	{
		FString WwiseCmd;
		FString CommandLineParams;
		GenerateCommandLineArguments(in_rBankNames, in_PlatformNames, WwiseCmd, CommandLineParams, WwisePathOverride);
		UE_LOG(LogAkBanks, Log, TEXT("Starting Wwise SoundBank generation with the following command line:"));
		UE_LOG(LogAkBanks, Log, TEXT("%s %s"), *WwiseCmd, *CommandLineParams);

		int32 ReturnCode;
		// Create a pipe for the child process's STDOUT.
		void* WritePipe;
		void* ReadPipe;
		FPlatformProcess::CreatePipe(ReadPipe, WritePipe);
		FProcHandle ProcHandle = FPlatformProcess::CreateProc(*WwiseCmd, *CommandLineParams, true, false, false, nullptr, 0, nullptr, WritePipe);
		if (ProcHandle.IsValid())
		{
			FString NewLine;
			FPlatformProcess::Sleep(0.1f);
			// Wait for it to finish and get return code
			while (FPlatformProcess::IsProcRunning(ProcHandle) == true)
			{
				NewLine = FPlatformProcess::ReadPipe(ReadPipe);
				if (NewLine.Len() > 0)
				{
					UE_LOG(LogAkBanks, Display, TEXT("%s"), *NewLine);
					NewLine.Empty();
				}
				FPlatformProcess::Sleep(0.25f);
			}

			NewLine = FPlatformProcess::ReadPipe(ReadPipe);
			if (NewLine.Len() > 0)
			{
				UE_LOG(LogAkBanks, Display, TEXT("%s"), *NewLine);
			}

			FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);

			switch (ReturnCode)
			{
			case 2:
				UE_LOG(LogAkBanks, Warning, TEXT("Wwise command-line completed with warnings."));
				break;
			case 0:
				UE_LOG(LogAkBanks, Display, TEXT("Wwise command-line successfully completed."));
				break;
			default:
				UE_LOG(LogAkBanks, Error, TEXT("Wwise command-line failed with error %d."), ReturnCode);
				break;
			}
		}
		else
		{
			ReturnCode = -1;
			// Most chances are the path to the .exe or the project were not set properly in GEditorIni file.
			UE_LOG(LogAkBanks, Error, TEXT("Failed to run Wwise command-line: %s %s"), *WwiseCmd, *CommandLineParams);
		}

		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);

		return ReturnCode;
	}

	void GenerateSoundBanksNonBlocking(TArray< TSharedPtr<FString> >& in_rBankNames, TArray< TSharedPtr<FString> >& in_PlatformNames, FOnBankGenerationComplete OnSoundBankGenerationComplete)
	{
		if (in_rBankNames.Num())
		{
			FString WwiseCmd;
			FString CommandLineParams;
			GenerateCommandLineArguments(in_rBankNames, in_PlatformNames, WwiseCmd, CommandLineParams);

			// To get more information on how banks can be generated from the comand lines.
			// Refer to the section: Generating Banks from the Command Line in the Wwise SDK documentation.
			UE_LOG(LogAkBanks, Log, TEXT("Starting Wwise SoundBank generation with the following command line:"));
			UE_LOG(LogAkBanks, Log, TEXT("%s %s"), *WwiseCmd, *CommandLineParams);

			//TSharedPtr<FMonitoredProcess> WwiseCliProcess = MakeShareable(new FMonitoredProcess(wwiseCmd, CommandLineParams, true));
			TSharedPtr<FMonitoredProcess> WwiseCliProcess = MakeShareable(new FMonitoredProcess(WwiseCmd, CommandLineParams, true));
			FNotificationInfo Info(LOCTEXT("GeneratingSoundBanks", "Generating SoundBanks..."));

			Info.Image = FAkAudioStyle ::GetBrush(TEXT("AudiokineticTools.AkPickerTabIcon"));
			Info.bFireAndForget = false;
			Info.FadeOutDuration = 0.0f;
			Info.ExpireDuration = 0.0f;
			Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog")); });
			Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
			Info.ButtonDetails.Add(
				FNotificationButtonInfo(
					LOCTEXT("UatTaskCancel", "Cancel"),
					LOCTEXT("UatTaskCancelToolTip", "Cancels execution of this task."),
					FSimpleDelegate::CreateLambda([=]()
			{
				if (WwiseCliProcess.IsValid())
				{
					WwiseCliProcess->Cancel(true);
				}
			}),
					SNotificationItem::CS_Pending
				)
			);
			Info.ButtonDetails.Add(
				FNotificationButtonInfo(
					LOCTEXT("UatTaskDismiss", "Dismiss"),
					FText(),
					FSimpleDelegate::CreateStatic(&FSoundBanksNotificationHandler::HandleDismissButtonClicked),
					SNotificationItem::CS_Fail
				)
			);

			TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

			if (!NotificationItem.IsValid())
			{
				return;
			}

			NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);

			// launch the packager
			TWeakPtr<SNotificationItem> NotificationItemPtr(NotificationItem);

			WwiseCliProcess->OnCanceled().BindStatic(&HandleWwiseCliProcessCanceled, NotificationItemPtr, OnSoundBankGenerationComplete);
			WwiseCliProcess->OnCompleted().BindStatic(&HandleWwiseCliProcessCompleted, NotificationItemPtr, OnSoundBankGenerationComplete);
			WwiseCliProcess->OnOutput().BindStatic(&ProcessWwiseCliOutput, NotificationItemPtr);

			if (WwiseCliProcess->Launch())
			{
				GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
			}
			else
			{
				GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));

				NotificationItem->SetText(LOCTEXT("UatLaunchFailedNotification", "Failed to launch Unreal Automation Tool (UAT)!"));

				NotificationItem->SetExpireDuration(3.0f);
				NotificationItem->SetFadeOutDuration(0.5f);
				NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
				NotificationItem->ExpireAndFadeout();
			}
		}
	}

	// Gets most recently modified JSON SoundBank metadata file
	struct BankNameToPath : private IPlatformFile::FDirectoryVisitor
	{
		FString BankPath;

		BankNameToPath(const FString& BankName, const TCHAR* BaseDirectory, IPlatformFile& PlatformFile)
			: BankName(BankName), PlatformFile(PlatformFile)
		{
			Visit(BaseDirectory, true);
			PlatformFile.IterateDirectory(BaseDirectory, *this);
		}

		bool IsValid() const { return StatData.bIsValid; }

	private:
		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (bIsDirectory)
			{
				FString NewBankPath = FilenameOrDirectory;
				NewBankPath += TEXT("\\");
				NewBankPath += BankName + TEXT(".json");

				FFileStatData NewStatData = PlatformFile.GetStatData(*NewBankPath);
				if (NewStatData.bIsValid && !NewStatData.bIsDirectory)
				{
					if (!StatData.bIsValid || NewStatData.ModificationTime > StatData.ModificationTime)
					{
						StatData = NewStatData;
						BankPath = NewBankPath;
					}
				}
			}

			return true;
		}

		const FString& BankName;
		IPlatformFile& PlatformFile;
		FFileStatData StatData;
	};

	void FetchAttenuationInfo(const TMap<FString, TSet<UAkAudioEvent*> >& BankToEventSet)
	{
		FString PlatformName = GetTargetPlatformManagerRef().GetRunningTargetPlatform()->PlatformName();
		FString BankBasePath = WwiseBnkGenHelper::GetBankGenerationFullDirectory(*PlatformName);
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		const TCHAR* BaseDirectory = *BankBasePath;

		FString FileContents; // cache the file contents - in case we are opening large files

		for (TMap<FString, TSet<UAkAudioEvent*> >::TConstIterator BankIt(BankToEventSet); BankIt; ++BankIt)
		{
			FString BankName = BankIt.Key();
			BankNameToPath NameToPath(BankName, BaseDirectory, PlatformFile);

			if (NameToPath.IsValid())
			{
				const TCHAR* BankPath = *NameToPath.BankPath;
				const TSet<UAkAudioEvent*>& EventsInBank = BankIt.Value();

				FileContents.Reset();
				if (!FFileHelper::LoadFileToString(FileContents, BankPath))
				{
					UE_LOG(LogAkBanks, Warning, TEXT("Failed to load file contents of JSON SoundBank metadata file: %s"), BankPath);
					continue;
				}

				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);

				if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
				{
					UE_LOG(LogAkBanks, Warning, TEXT("Unable to parse JSON SoundBank metadata file: %s"), BankPath);
					continue;
				}

				const TSharedPtr<FJsonObject>* SoundBanksInfo = nullptr;
				if (!JsonObject->TryGetObjectField("SoundBanksInfo", SoundBanksInfo))
				{
					UE_LOG(LogAkBanks, Warning, TEXT("Malformed JSON SoundBank metadata file: %s"), BankPath);
					continue;
				}

				const TArray<TSharedPtr<FJsonValue>>* SoundBanks = nullptr;
				if (!(*SoundBanksInfo)->TryGetArrayField("SoundBanks", SoundBanks))
				{
					UE_LOG(LogAkBanks, Warning, TEXT("Malformed JSON SoundBank metadata file: %s"), BankPath);
					continue;
				}

				TSharedPtr<FJsonObject> Obj = (*SoundBanks)[0]->AsObject();
				const TArray<TSharedPtr<FJsonValue>>* Events;
				if (!Obj->TryGetArrayField("IncludedEvents", Events))
				{
					// If we get here, it is because we are parsing a SoundBank that has no events - possibly containing external sources
					continue;
				}

				for (int i = 0; i < Events->Num(); i++)
				{
					TSharedPtr<FJsonObject> EventObj = (*Events)[i]->AsObject();

					FString EventName;
					if (!EventObj->TryGetStringField("Name", EventName))
						continue;

					UAkAudioEvent* Event = nullptr;
					for (auto TestEvent : EventsInBank)
					{
						if (TestEvent->GetName() == EventName)
						{
							Event = TestEvent;
							break;
						}
					}

					if (Event == nullptr)
						continue;

					bool Changed = false;
					FString ValueString;
					if (EventObj->TryGetStringField("MaxAttenuation", ValueString))
					{
						const float EventRadius = FCString::Atof(*ValueString);
						if (Event->MaxAttenuationRadius != EventRadius)
						{
							Event->MaxAttenuationRadius = EventRadius;
							Changed = true;
						}
					}
					else if (Event->MaxAttenuationRadius != 0)
					{
						// No attenuation info in json file, set to 0.
						Event->MaxAttenuationRadius = 0;
						Changed = true;
					}

					// if we can't find "DurationType", then we assume infinite
					const bool IsInfinite = !EventObj->TryGetStringField("DurationType", ValueString) || (ValueString == "Infinite") || (ValueString == "Unknown");
					if (Event->IsInfinite != IsInfinite)
					{
						Event->IsInfinite = IsInfinite;
						Changed = true;
					}

					if (!IsInfinite)
					{
						if (EventObj->TryGetStringField("DurationMin", ValueString))
						{
							const float DurationMin = FCString::Atof(*ValueString);
							if (Event->MinimumDuration != DurationMin)
							{
								Event->MinimumDuration = DurationMin;
								Changed = true;
							}
						}

						if (EventObj->TryGetStringField("DurationMax", ValueString))
						{
							const float DurationMax = FCString::Atof(*ValueString);
							if (Event->MaximumDuration != DurationMax)
							{
								Event->MaximumDuration = DurationMax;
								Changed = true;
							}
						}
					}

					if (Changed)
					{
						Event->Modify(true);
					}
				}
			}
		}
	}

	void CreateGenerateSoundBankWindow(TArray<TWeakObjectPtr<UAkAudioBank>>* pSoundBanks, bool in_bShouldSaveWwiseProject/* = false*/)
	{
		TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
			.Title(LOCTEXT("AkAudioGenerateSoundBanks", "Generate SoundBanks"))
			//.CenterOnScreen(true)
			.ClientSize(FVector2D(600.f, 332.f))
			.SupportsMaximize(false).SupportsMinimize(false)
			.SizingRule(ESizingRule::FixedSize)
			.FocusWhenFirstShown(true);

		TSharedRef<SGenerateSoundBanks> WindowContent = SNew(SGenerateSoundBanks, pSoundBanks);
		WindowContent->SetShouldSaveWwiseProject(in_bShouldSaveWwiseProject);
		if (!WindowContent->ShouldDisplayWindow())
		{
			return;
		}

		// Add our SGenerateSoundBanks to the window
		WidgetWindow->SetContent(WindowContent);

		// Set focus to our SGenerateSoundBanks widget, so our keyboard keys work right off the bat
		WidgetWindow->SetWidgetToFocusOnActivate(WindowContent);

		// This creates a windows that blocks the rest of the UI. You can only interact with the "Generate SoundBanks" window.
		// If you choose to use this, comment the rest of the function.
		//GEditor->EditorAddModalWindow(WidgetWindow);

		// This creates a window that still allows you to interact with the rest of the editor. If there is an attempt to delete
		// a UAkAudioBank (from the content browser) while this window is opened, the editor will generate a (cryptic) error message
		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		if (ParentWindow.IsValid())
		{
			// Parent the window to the main frame 
			FSlateApplication::Get().AddWindowAsNativeChild(WidgetWindow, ParentWindow.ToSharedRef());
		}
		else
		{
			// Spawn new window
			FSlateApplication::Get().AddWindow(WidgetWindow);
		}
	}

}

#undef LOCTEXT_NAMESPACE
