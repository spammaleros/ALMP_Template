#pragma once

#if PLATFORM_ANDROID && !PLATFORM_LUMIN

#include "AkInclude.h"
#include "AkAndroidInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkAndroidInitializationSettings;

struct FAkAndroidPlatform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkAndroidInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Android");
	}
};

using FAkPlatform = FAkAndroidPlatform;

#endif
