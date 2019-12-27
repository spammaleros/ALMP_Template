#pragma once

#include "Platforms/AkPlatformInfo.h"
#include "AkXboxOnePlatformInfo.generated.h"

UCLASS()
class UAkXboxOnePlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkXboxOnePlatformInfo()
	{
		FString VSVersion = "vc140";
#ifdef AK_XBOXONE_VS_VERSION
		VSVersion = AK_XBOXONE_VS_VERSION;
#endif 

		WwisePlatform = "XboxOne";
		Architecture = FString("XboxOne_") + VSVersion;
		LibraryFileNameFormat = "{0}.dll";
		DebugFileNameFormat = "{0}.pdb";
	}
};
