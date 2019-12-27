// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved
#include "AkUnrealHelper.h"

// Unreal includes
#include "AkUEFeatures.h"
#include "AkSettings.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "HAL/FileManager.h"
#include "Widgets/Docking/SDockTab.h"
#endif

namespace AkUnrealHelper
{
	void TrimPath(FString& Path)
	{
#if UE_4_18_OR_LATER
		Path.TrimStartAndEndInline();
#else
		Path = Path.Trim();
		Path = Path.TrimTrailing();
#endif // UE_4_18_OR_LATER
	}

	FString GetProjectDirectory()
	{
#if UE_4_18_OR_LATER
		return FPaths::ProjectDir();
#else
		return FPaths::GameDir();
#endif // UE_4_18_OR_LATER
	}

	FString GetContentDirectory()
	{
#if UE_4_18_OR_LATER
		return FPaths::ProjectContentDir();
#else
		return FPaths::GameContentDir();
#endif // UE_4_18_OR_LATER
	}

	FString GetSoundBankDirectory()
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();

		if (AkSettings)
		{
			return FPaths::Combine(GetContentDirectory(), AkSettings->WwiseSoundBankFolder.Path);
		}
		else
		{
			return FPaths::Combine(GetContentDirectory(), UAkSettings::DefaultSoundBankFolder);
		}
	}

#if WITH_EDITOR
	void SanitizePath(FString& Path, const FString& PreviousPath, const FText& DialogMessage)
	{
		AkUnrealHelper::TrimPath(Path);

		FText FailReason;
		if (!FPaths::ValidatePath(Path, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			Path = PreviousPath;
		}

		const FString TempPath = FPaths::IsRelative(Path) ? FPaths::ConvertRelativePathToFull(AkUnrealHelper::GetProjectDirectory(), Path) : Path;
		if (!FPaths::DirectoryExists(TempPath))
		{
			FMessageDialog::Open(EAppMsgType::Ok, DialogMessage);
			Path = PreviousPath;
		}
	}

	void SanitizeProjectPath(FString& Path, const FString& PreviousPath, const FText& DialogMessage, bool &bRequestRefresh)
	{
		AkUnrealHelper::TrimPath(Path);

		FString TempPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*Path);

		FText FailReason;
		if (!FPaths::ValidatePath(TempPath, &FailReason))
		{
			if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, FailReason))
			{
				Path = PreviousPath;
				return;
			}
		}

		auto ProjectDirectory = AkUnrealHelper::GetProjectDirectory();
		if (!FPaths::FileExists(TempPath))
		{
			// Path might be a valid one (relative to game) entered manually. Check that.
			TempPath = FPaths::ConvertRelativePathToFull(ProjectDirectory, Path);

			if (!FPaths::FileExists(TempPath))
			{
				if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, DialogMessage))
				{
					Path = PreviousPath;
					return;
				}
			}
		}

		// Make the path relative to the game dir
		FPaths::MakePathRelativeTo(TempPath, *ProjectDirectory);
		Path = TempPath;

		if (Path != PreviousPath)
		{
			TSharedRef<SDockTab> WaapiPickerTab = FGlobalTabmanager::Get()->InvokeTab(FName("WaapiPicker"));
			TSharedRef<SDockTab> WwisePickerTab = FGlobalTabmanager::Get()->InvokeTab(FName("WwisePicker"));
			bRequestRefresh = true;
		}
	}
#endif
}
