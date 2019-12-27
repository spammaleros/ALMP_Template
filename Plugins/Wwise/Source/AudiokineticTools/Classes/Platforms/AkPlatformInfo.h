#pragma once

#include "Engine/GameEngine.h"
#include "AkPlatformInfo.generated.h"

UCLASS()
class UAkPlatformInfo : public UObject
{
	GENERATED_BODY()

public:
	static UAkPlatformInfo* GetAkPlatformInfo(const FString& PlatformName)
	{
		FString PlatformInfoClassName = FString::Format(TEXT("Ak{0}PlatformInfo"), { *PlatformName });
		auto* PlatformInfoClass = FindObject<UClass>(ANY_PACKAGE, *PlatformInfoClassName);
		if (!PlatformInfoClass)
		{
			return nullptr;
		}

		return PlatformInfoClass->GetDefaultObject<UAkPlatformInfo>();
	}

	FString WwisePlatform;
	FString Architecture;
	FString LibraryFileNameFormat;
	FString DebugFileNameFormat;
	bool bSupportsUPL = false;
	bool bUsesStaticLibraries = false;
	bool bForceReleaseConfig = false;
};
