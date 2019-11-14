// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "Platforms/AkPlatform_iOS/AkIOSInitializationSettings.h"
#include "AkAudioDevice.h"

#if PLATFORM_IOS
#include <AK/Plugin/AkAACFactory.h>
#endif

//////////////////////////////////////////////////////////////////////////
// FAkAudioSession

void FAkAudioSession::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#if PLATFORM_IOS || PLATFORM_TVOS
	InitializationStructure.PlatformInitSettings.audioSession.eCategory = (AkAudioSessionCategory)AudioSessionCategory;
	InitializationStructure.PlatformInitSettings.audioSession.eCategoryOptions = (AkAudioSessionCategoryOptions)AudioSessionCategoryOptions;
	InitializationStructure.PlatformInitSettings.audioSession.eMode = (AkAudioSessionMode)AudioSessionMode;
#endif
}


//////////////////////////////////////////////////////////////////////////
// UAkIOSInitializationSettings

UAkIOSInitializationSettings::UAkIOSInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CommonSettings.DefaultPoolSize = 16 * 1024 * 1024;
	CommonSettings.MainOutputSettings.PanningRule = EAkPanningRule::Headphones;
	CommonSettings.MainOutputSettings.ChannelConfigType = EAkChannelConfigType::Standard;
	CommonSettings.MainOutputSettings.ChannelMask = AK_SPEAKER_SETUP_STEREO;
}

void UAkIOSInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	AudioSession.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if !PLATFORM_MAC && PLATFORM_APPLE
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
	// From FRunnableThreadApple
	InitializationStructure.DeviceSettings.threadProperties.uStackSize = 256 * 1024;
#endif
}
