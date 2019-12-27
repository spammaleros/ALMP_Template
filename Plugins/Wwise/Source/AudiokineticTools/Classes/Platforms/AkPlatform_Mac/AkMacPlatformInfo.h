#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkMacPlatformInfo.generated.h"

UCLASS()
class UAkMacPlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkMacPlatformInfo()
	{
		WwisePlatform = "Mac";
		Architecture = "Mac";
		LibraryFileNameFormat = "lib{0}.dylib";
	}
};
