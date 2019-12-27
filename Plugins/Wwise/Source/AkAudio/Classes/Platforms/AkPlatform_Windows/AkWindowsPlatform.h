#pragma once

#if PLATFORM_WINDOWS

#include "AkInclude.h"
#include "AkWindowsInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const WIDECHAR*)(Text)

using UAkInitializationSettings = UAkWindowsInitializationSettings;

struct FAkWindowsPlatform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkWindowsInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Windows");
	}
};

using FAkPlatform = FAkWindowsPlatform;

#endif
