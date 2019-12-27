#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkLuminPlatformInfo.generated.h"

UCLASS()
class UAkLuminPlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkLuminPlatformInfo()
	{
		WwisePlatform = "Lumin";
		bSupportsUPL = true;
	}
};
