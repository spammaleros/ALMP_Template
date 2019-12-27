// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkIncludes.h:
=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#pragma once
#include "Engine/Engine.h"

#if PLATFORM_WINDOWS

// Currently, Wwise SDK builds with the default 8 byte alignment, whereas Unreal builds with 4 byte alignment under VC.
// This causes data corruption if the headers are not included with forced alignment directives.
// http://msdn.microsoft.com/en-us/library/xh3e3fd0%28VS.80%29.aspx
#pragma pack(push, 8)
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/MinimalWindowsApi.h"
#elif PLATFORM_XBOXONE
#include "XboxOne/XboxOneMinApi.h"
#endif

#include "AkUEFeatures.h"

#include "CoreTypes.h" // includes platform defines

#include <AK/AkWwiseSDKVersion.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>
#include <AK/IBytes.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkMonitorError.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#ifndef AK_OPTIMIZED
    #include <AK/Comm/AkCommunication.h>
#endif // AK_OPTIMIZED

#if defined AK_SOUNDFRAME
	#include <AK/SoundFrame/SF.h>
#endif
#include <AK/SoundEngine/Common/AkQueryParameters.h>

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformAtomics.h"
#pragma pack(pop)
#endif // PLATFORM_WINDOWS
