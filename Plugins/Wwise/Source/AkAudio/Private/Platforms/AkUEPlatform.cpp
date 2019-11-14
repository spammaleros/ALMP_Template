// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved


#include "Platforms/AkUEPlatform.h"
#include "AkAudioDevice.h"
#include "Interfaces/IProjectManager.h"
#include "ProjectDescriptor.h"

#if WITH_EDITOR
#include "PlatformInfo.h"
TSet<FString> AkUnrealPlatformHelper::GetAllSupportedUnrealPlatforms()
{
	TSet<FString> SupportedPlatforms;
#if UE_4_23_OR_LATER
	for (const PlatformInfo::FPlatformInfo& Info : PlatformInfo::GetPlatformInfoArray())
#else
	for (const PlatformInfo::FPlatformInfo& Info : PlatformInfo::EnumeratePlatformInfoArray())
#endif
	{
		if (Info.IsVanilla() && (Info.PlatformType == PlatformInfo::EPlatformType::Game) && (Info.PlatformInfoName != TEXT("AllDesktop")))
		{
			FString VanillaName = Info.VanillaPlatformName.ToString();
			VanillaName.RemoveFromEnd(TEXT("NoEditor"));
			SupportedPlatforms.Add(VanillaName);
		}
	}

	return SupportedPlatforms;
}

TSet<FString> AkUnrealPlatformHelper::GetAllSupportedUnrealPlatformsForProject()
{
	TSet<FString> SupportedPlatforms = GetAllSupportedUnrealPlatforms();
	IProjectManager& ProjectManager = IProjectManager::Get();
	auto* CurrentProject = ProjectManager.GetCurrentProject();
	if (CurrentProject && CurrentProject->TargetPlatforms.Num() > 0)
	{
		auto& TargetPlatforms = CurrentProject->TargetPlatforms;
		TSet<FString> AvailablePlatforms;
		for (const auto& TargetPlatform : TargetPlatforms)
		{
			FString PlatformName = TargetPlatform.ToString();
			PlatformName.RemoveFromEnd(TEXT("NoEditor"));
			AvailablePlatforms.Add(PlatformName);
		}

		auto Intersection = SupportedPlatforms.Intersect(AvailablePlatforms);
		if (Intersection.Num() > 0)
		{
			SupportedPlatforms = Intersection;
		}
	}

	return SupportedPlatforms;
}

TArray<TSharedPtr<FString> > AkUnrealPlatformHelper::GetAllSupportedWwisePlatforms(bool ProjectScope /* = false */)
{
	TSet<FString> UnrealPlatforms;
	if (ProjectScope)
	{
		UnrealPlatforms = GetAllSupportedUnrealPlatformsForProject();
	}
	else
	{
		UnrealPlatforms = GetAllSupportedUnrealPlatforms();
	}

	TSet<FString> TemporaryWwisePlatformNames;
	for (const auto& AvailablePlatform : UnrealPlatforms)
	{
		FString SettingsClassName = FString::Format(TEXT("Ak{0}InitializationSettings"), { *AvailablePlatform });
		if(FindObject<UClass>(ANY_PACKAGE, *SettingsClassName))
		{
			TemporaryWwisePlatformNames.Add(AvailablePlatform);
		}
	}

	TemporaryWwisePlatformNames.Sort([&](const FString& L, const FString& R) { return L.Compare(R) < 0; });

	TArray<TSharedPtr<FString> > WwisePlatforms;
	for (const auto WwisePlatformName : TemporaryWwisePlatformNames)
	{
		WwisePlatforms.Add(TSharedPtr<FString>(new FString(WwisePlatformName)));
	}

	return WwisePlatforms;
}
#endif

