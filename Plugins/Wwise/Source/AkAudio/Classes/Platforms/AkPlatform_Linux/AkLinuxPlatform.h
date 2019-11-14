#pragma once

#if PLATFORM_LINUX

#include "AkInclude.h"
#include "AkLinuxInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkLinuxInitializationSettings;

struct FAkLinuxPlatform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkLinuxInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Linux");
	}
};

using FAkPlatform = FAkLinuxPlatform;

#endif
