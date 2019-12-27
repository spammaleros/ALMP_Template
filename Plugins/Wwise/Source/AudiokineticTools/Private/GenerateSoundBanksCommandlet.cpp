// Fill out your copyright notice in the Description page of Project Settings.

#include "GenerateSoundBanksCommandlet.h"

#if WITH_EDITOR
#include "AssetRegistryModule.h"
#include "Editor.h"
#include "AkAudioBank.h"
#include "AkAudioEvent.h"
#include "AkAudioBankGenerationHelpers.h"

#define LOCTEXT_NAMESPACE "AkAudio"
DEFINE_LOG_CATEGORY_STATIC(LogAkBanksCommandlet, Log, All);
#endif

UGenerateSoundBanksCommandlet::UGenerateSoundBanksCommandlet()
{
	IsClient = false;
	IsEditor = false;
	IsServer = false;
	LogToConsole = true;

	HelpDescription = TEXT("Commandlet allowing to generate Wwise SoundBanks.");

	HelpParamNames.Add(TEXT("platforms"));
	HelpParamDescriptions.Add(TEXT("(Optional) Comma separated list of platforms for which SoundBanks will be generated, as specified in the Wwise project. If not specified, SoundBanks will be generated for all platforms."));

	HelpParamNames.Add(TEXT("banks"));
	HelpParamDescriptions.Add(TEXT("(Optional) Comma separated list of SoundBanks to generate. Bank names must correspond to a UAkAudioBank asset in the project. If now specified, all SoundBanks found in project will be generated."));

	HelpParamNames.Add(TEXT("wwiseCliPath"));
	HelpParamDescriptions.Add(TEXT("(Optional) Full path to the Wwise command-line application to use to generate the SoundBanks. If not specified, the path found in the Wwise settings will be used."));

	HelpParamNames.Add(TEXT("help"));
	HelpParamDescriptions.Add(TEXT("(Optional) Print this help message. This will quit the commandlet immediately."));

	HelpUsage = TEXT("<Editor.exe> <path_to_uproject> -run=GenerateSoundBanks [-platforms=listOfPlatforms] [-banks=listOfBanks] [-wwiseCliPath=pathToWwiseCli]");
	HelpWebLink = TEXT("https://www.audiokinetic.com/library/edge/?source=UE4&id=using_features_generatecommandlet.html");
}

void UGenerateSoundBanksCommandlet::PrintHelp() const
{
	UE_LOG(LogAkBanksCommandlet, Display, TEXT("%s"), *HelpDescription);
	UE_LOG(LogAkBanksCommandlet, Display, TEXT("Usage: %s"), *HelpUsage);
	UE_LOG(LogAkBanksCommandlet, Display, TEXT("Parameters:"));
	for (int32 i = 0; i < HelpParamNames.Num(); ++i)
	{
		UE_LOG(LogAkBanksCommandlet, Display, TEXT("\t- %s: %s"), *HelpParamNames[i], *HelpParamDescriptions[i]);
	}
	UE_LOG(LogAkBanksCommandlet, Display, TEXT("For more information, see %s"), *HelpWebLink);
}

int32 UGenerateSoundBanksCommandlet::Main(const FString& Params)
{
	int32 ReturnCode = -1;
#if WITH_EDITOR
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray< TSharedPtr<FString> > BanksToGenerate;
	TArray< TSharedPtr<FString> > PlatformsToGenerate;

	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamVals;

	ParseCommandLine(*Params, Tokens, Switches, ParamVals);

	if (Switches.Contains("help"))
	{
		PrintHelp();
		return 0;
	}

	const FString* WwiseCliPath = ParamVals.Find(FString(TEXT("wwiseCliPath")));

	TArray<FString> PlatformNames;
	const FString* PlatformValue = ParamVals.Find(FString(TEXT("platforms")));
	if (PlatformValue)
	{
		PlatformValue->ParseIntoArray(PlatformNames, TEXT(","));
		for (FString name : PlatformNames)
			PlatformsToGenerate.Add(MakeShareable(new FString(name)));
	}

	TArray<FString> BanksGivenAsParam;
	const FString* BankValue = ParamVals.Find(FString(TEXT("banks")));
	if(BankValue)
		BankValue->ParseIntoArray(BanksGivenAsParam, TEXT(","));

	bool GenerateAllBanks = BanksGivenAsParam.Num() == 0;

	// Force load of bank assets so that the list is fully populated.
	{
		TArray<FAssetData> BankAssets;
		TArray<FString> PathsToScan;
		PathsToScan.Add(TEXT("/Game/"));
		AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan);
		AssetRegistryModule.Get().GetAssetsByClass(UAkAudioBank::StaticClass()->GetFName(), BankAssets);

		for (int32 AssetIndex = 0; AssetIndex < BankAssets.Num(); ++AssetIndex)
		{
			FString currentBankName(BankAssets[AssetIndex].AssetName.ToString());
			if (BanksGivenAsParam.Contains(currentBankName) || GenerateAllBanks)
			{
				BanksToGenerate.Add(MakeShareable(new FString(currentBankName)));
				BanksGivenAsParam.Remove(currentBankName);
			}
		}
	}

	if (BanksToGenerate.Num() == 0)
	{
		UE_LOG(LogAkBanksCommandlet, Error, TEXT("Found no SoundBanks to generate"));
		return -1;
	}

	if (BanksGivenAsParam.Num() != 0)
	{
		UE_LOG(LogAkBanksCommandlet, Error, TEXT("Could not find the following soundbanks: %s"), *FString::Join(BanksGivenAsParam, TEXT(", ")));
	}

	FString TempBankNames = *BanksToGenerate[0];
	for (int32 i = 1; i < BanksToGenerate.Num(); ++i)
		TempBankNames += TEXT(", ") + *BanksToGenerate[i];

	UE_LOG(LogAkBanksCommandlet, Display, TEXT("Generating %s SoundBanks for %s platforms using %s."), 
		*TempBankNames, 
		PlatformValue ? **PlatformValue : TEXT("all"),
		WwiseCliPath ? **WwiseCliPath : TEXT("WwiseCli found in project settings."));

	TMap<FString, TSet<UAkAudioEvent*> > BankToEventSet;
	bool SuccesfulDump = WwiseBnkGenHelper::GenerateDefinitionFile(BanksToGenerate, BankToEventSet);
	if (SuccesfulDump)
	{
		ReturnCode = WwiseBnkGenHelper::GenerateSoundBanksBlocking(BanksToGenerate, PlatformsToGenerate, WwiseCliPath);
		if (ReturnCode == 0 || ReturnCode == 2)
		{
			WwiseBnkGenHelper::FetchAttenuationInfo(BankToEventSet);
			ReturnCode = 0;
		}
		else
		{
			UE_LOG(LogAkBanksCommandlet, Error, TEXT("Failed to generate SoundBanks."));
		}
	}
	else
	{
		UE_LOG(LogAkBanksCommandlet, Error, TEXT("Failed to generate SoundBank definition file."));

	}
#endif
	return ReturnCode;
}

#undef LOCTEXT_NAMESPACE
