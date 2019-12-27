#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkAndroidPlatformInfo.generated.h"

UCLASS()
class UAkAndroidPlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkAndroidPlatformInfo()
	{
		WwisePlatform = "Android";
		bSupportsUPL = true;
	}
};

