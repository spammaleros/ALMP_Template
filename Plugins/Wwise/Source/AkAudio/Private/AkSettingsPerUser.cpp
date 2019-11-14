// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved
#include "AkSettingsPerUser.h"

#include "AkAudioDevice.h"
#include "Misc/Paths.h"
#include "AkUnrealHelper.h"

//////////////////////////////////////////////////////////////////////////
// UAkSettingsPerUser

UAkSettingsPerUser::UAkSettingsPerUser(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if UE_4_21_OR_LATER
	WwiseWindowsInstallationPath.Path = FPlatformMisc::GetEnvironmentVariable(TEXT("WWISEROOT"));
#else
	TCHAR WwiseDir[AK_MAX_PATH];
	FPlatformMisc::GetEnvironmentVariable(TEXT("WWISEROOT"), WwiseDir, AK_MAX_PATH);

	WwiseWindowsInstallationPath.Path = FString(WwiseDir);
#endif
}

#if WITH_EDITOR
void UAkSettingsPerUser::PreEditChange(UProperty* PropertyAboutToChange)
{
	PreviousWwiseWindowsInstallationPath = WwiseWindowsInstallationPath.Path;
	PreviousWwiseMacInstallationPath = WwiseMacInstallationPath.FilePath;
}

void UAkSettingsPerUser::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WwiseWindowsInstallationPath))
	{
		AkUnrealHelper::SanitizePath(WwiseWindowsInstallationPath.Path, PreviousWwiseWindowsInstallationPath, FText::FromString("Please enter a valid Wwise Installation path"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WwiseMacInstallationPath))
	{
		AkUnrealHelper::SanitizePath(WwiseMacInstallationPath.FilePath, PreviousWwiseMacInstallationPath, FText::FromString("Please enter a valid Wwise Authoring Mac executable path"));
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
