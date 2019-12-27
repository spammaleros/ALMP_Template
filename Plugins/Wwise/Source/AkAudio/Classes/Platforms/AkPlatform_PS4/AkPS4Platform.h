#pragma once

#if PLATFORM_PS4

#include "AkInclude.h"
#include "AkPS4InitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkPS4InitializationSettings;

struct FAkPS4Platform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkPS4InitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("PS4");
	}
};

using FAkPlatform = FAkPS4Platform;

#endif
