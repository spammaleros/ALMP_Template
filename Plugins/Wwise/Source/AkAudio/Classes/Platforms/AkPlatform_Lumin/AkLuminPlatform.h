#pragma once

#if PLATFORM_LUMIN

#include "AkInclude.h"
#include "AkLuminInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkLuminInitializationSettings;

struct FAkLuminPlatform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkLuminInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Lumin");
	}
};

using FAkPlatform = FAkLuminPlatform;

#endif
