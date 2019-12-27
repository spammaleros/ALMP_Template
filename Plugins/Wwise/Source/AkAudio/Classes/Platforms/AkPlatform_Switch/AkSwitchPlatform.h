#pragma once

#if PLATFORM_SWITCH

#include "AkInclude.h"
#include "AkSwitchInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkSwitchInitializationSettings;

struct FAkSwitchPlatform
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkSwitchInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Switch");
	}
};

using FAkPlatform = FAkSwitchPlatform;

#endif
