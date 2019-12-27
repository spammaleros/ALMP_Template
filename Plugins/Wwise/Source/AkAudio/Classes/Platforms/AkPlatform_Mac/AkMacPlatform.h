#pragma once

#if PLATFORM_MAC

#include "AkInclude.h"
#include "AkMacInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkMacInitializationSettings;

struct FAkMacPlatform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkMacInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Mac");
	}
};

using FAkPlatform = FAkMacPlatform;

#endif
