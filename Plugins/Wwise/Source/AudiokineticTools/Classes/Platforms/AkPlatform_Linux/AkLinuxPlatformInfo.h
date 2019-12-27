#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkLinuxPlatformInfo.generated.h"

UCLASS()
class UAkLinuxPlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkLinuxPlatformInfo()
	{
		WwisePlatform = "Linux";
		Architecture = "Linux_x64";
		LibraryFileNameFormat = "lib{0}.so";
		bForceReleaseConfig = true;
	}
};
