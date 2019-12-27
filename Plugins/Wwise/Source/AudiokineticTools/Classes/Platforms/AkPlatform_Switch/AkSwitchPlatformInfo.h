#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkSwitchPlatformInfo.generated.h"

UCLASS()
class UAkSwitchPlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkSwitchPlatformInfo()
	{
		WwisePlatform = "Switch";
		bUsesStaticLibraries = true;
	}
};
