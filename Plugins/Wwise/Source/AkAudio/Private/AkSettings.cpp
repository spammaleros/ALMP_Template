// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "AkSettings.h"
#include "AkAudioDevice.h"
#include "AkUnrealHelper.h"
#include "AkSettingsPerUser.h"

#if WITH_EDITOR
#include "Settings/ProjectPackagingSettings.h"
#include "Platforms/AkUEPlatform.h"
#include "InitializationSettings/AkInitializationSettings.h"
#endif

#include "UObject/UnrealType.h"

//////////////////////////////////////////////////////////////////////////
// UAkSettings

namespace AkSettings_Helper
{
	void MigrateMultiCoreRendering(bool EnableMultiCoreRendering, const FString& PlatformName)
	{
		FString SettingsClassName = FString::Format(TEXT("Ak{0}InitializationSettings"), { *PlatformName });
		auto* SettingsClass = FindObject<UClass>(ANY_PACKAGE, *SettingsClassName);
		if (!SettingsClass)
		{
			return;
		}

		auto* MigrationFunction = SettingsClass->FindFunctionByName(TEXT("MigrateMultiCoreRendering"));
		auto* Settings = SettingsClass->GetDefaultObject();
		if (!MigrationFunction || !Settings)
		{
			return;
		}

		Settings->ProcessEvent(MigrationFunction, &EnableMultiCoreRendering);
		Settings->UpdateDefaultConfigFile();
	}
}

FString UAkSettings::DefaultSoundBankFolder = TEXT("WwiseAudio");

UAkSettings::UAkSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WwiseSoundBankFolder.Path = DefaultSoundBankFolder;
}

void UAkSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();

	if (AkSettingsPerUser)
	{
		bool didChanges = false;

		if (!WwiseWindowsInstallationPath_DEPRECATED.Path.IsEmpty())
		{
			AkSettingsPerUser->WwiseWindowsInstallationPath = WwiseWindowsInstallationPath_DEPRECATED;
			WwiseWindowsInstallationPath_DEPRECATED.Path.Reset();
			didChanges = true;
		}

		if (!WwiseMacInstallationPath_DEPRECATED.FilePath.IsEmpty())
		{
			AkSettingsPerUser->WwiseMacInstallationPath = WwiseMacInstallationPath_DEPRECATED;
			WwiseMacInstallationPath_DEPRECATED.FilePath.Reset();
			didChanges = true;
		}

		if (didChanges)
		{
			UpdateDefaultConfigFile();
			AkSettingsPerUser->SaveConfig();
		}
	}

	if (!MigratedEnableMultiCoreRendering)
	{
		MigratedEnableMultiCoreRendering = true;

		for (const auto& PlatformName : AkUnrealPlatformHelper::GetAllSupportedWwisePlatforms())
		{
			AkSettings_Helper::MigrateMultiCoreRendering(bEnableMultiCoreRendering_DEPRECATED, *PlatformName);
		}
	}
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void UAkSettings::PreEditChange(UProperty* PropertyAboutToChange)
{
	PreviousWwiseProjectPath = WwiseProjectPath.FilePath;
	PreviousSoundBankFolder = WwiseSoundBankFolder.Path;
}

void UAkSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if ( PropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, MaxSimultaneousReverbVolumes) )
	{
		MaxSimultaneousReverbVolumes = FMath::Clamp<uint8>( MaxSimultaneousReverbVolumes, 0, AK_MAX_AUX_PER_OBJ );
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if( AkAudioDevice )
		{
			AkAudioDevice->SetMaxAuxBus(MaxSimultaneousReverbVolumes);
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, WwiseProjectPath))
	{
		AkUnrealHelper::SanitizeProjectPath(WwiseProjectPath.FilePath, PreviousWwiseProjectPath, FText::FromString("Please enter a valid Wwise project"), bRequestRefresh);
	}
    else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, bAutoConnectToWAAPI))
    {
        OnAutoConnectChanged.Broadcast();
    }
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, WwiseSoundBankFolder))
	{
		EnsureSoundBankPathIsInPackagingSettings();
	}

	PreviousSoundBankFolder.Empty();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAkSettings::EnsureSoundBankPathIsInPackagingSettings() 
{
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	bool foundPackageDirectory = false;
	bool packageSettingsNeedUpdate = false;

	for (int32 i = 0; i < PackagingSettings->DirectoriesToAlwaysStageAsUFS.Num(); i++)
	{
		if (PackagingSettings->DirectoriesToAlwaysStageAsUFS[i].Path == WwiseSoundBankFolder.Path)
		{
			foundPackageDirectory = true;
			break;
		}
	}

	if (!foundPackageDirectory)
	{
		PackagingSettings->DirectoriesToAlwaysStageAsUFS.Add(WwiseSoundBankFolder);
		packageSettingsNeedUpdate = true;
	}

	if (!PreviousSoundBankFolder.IsEmpty())
	{
		for (int i = 0; i < PackagingSettings->DirectoriesToAlwaysStageAsUFS.Num(); i++)
		{
			if (PackagingSettings->DirectoriesToAlwaysStageAsUFS[i].Path == PreviousSoundBankFolder)
			{
				PackagingSettings->DirectoriesToAlwaysStageAsUFS.RemoveAt(i);
				packageSettingsNeedUpdate = true;
			}
		}
	}

	if (packageSettingsNeedUpdate)
	{
		PackagingSettings->UpdateDefaultConfigFile();
	}
}
#endif // WITH_EDITOR
