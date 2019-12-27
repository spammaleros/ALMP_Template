#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkWindowsPlatformInfo.generated.h"

UCLASS()
class UAkWin32PlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkWin32PlatformInfo()
	{
		FString VSVersion = "vc150";
#ifdef AK_WINDOWS_VS_VERSION
		VSVersion = AK_WINDOWS_VS_VERSION;
#endif 

		WwisePlatform = "Windows";
		Architecture = FString("Win32_") + VSVersion;
		LibraryFileNameFormat = "{0}.dll";
		DebugFileNameFormat = "{0}.pdb";
	}
};

UCLASS()
class UAkWin64PlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkWin64PlatformInfo()
	{
		FString VSVersion = "vc150";
#ifdef AK_WINDOWS_VS_VERSION
		VSVersion = AK_WINDOWS_VS_VERSION;
#endif 

		WwisePlatform = "Windows";
		Architecture = FString("x64_") + VSVersion;
		LibraryFileNameFormat = "{0}.dll";
		DebugFileNameFormat = "{0}.pdb";
	}
};

UCLASS()
class UAkWindowsPlatformInfo : public UAkWin64PlatformInfo
{
	GENERATED_BODY()
};
