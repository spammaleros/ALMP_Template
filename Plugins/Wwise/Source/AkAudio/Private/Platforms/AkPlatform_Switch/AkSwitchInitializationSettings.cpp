// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "Platforms/AkPlatform_Switch/AkSwitchInitializationSettings.h"
#include "AkAudioDevice.h"
#include "Misc/Paths.h"

#if PLATFORM_SWITCH
#include <AK/Plugin/AkOpusNXFactory.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Memory hooks

namespace AK
{
#if PLATFORM_SWITCH
	void* AlignedAllocHook(size_t in_size, size_t in_alignment)
	{
		return aligned_alloc(in_alignment, in_size);
	}

	void AlignedFreeHook(void* in_ptr)
	{
		free(in_ptr);
	}
#endif // PLATFORM_SWITCH
}

//////////////////////////////////////////////////////////////////////////
// UAkSwitchInitializationSettings

UAkSwitchInitializationSettings::UAkSwitchInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CommunicationSettings.DiscoveryBroadcastPort = FAkCommunicationSettings::DefaultDiscoveryBroadcastPort;
	CommunicationSettings.CommandPort = FAkCommunicationSettings::DefaultDiscoveryBroadcastPort + 1;
	CommunicationSettings.NotificationPort = FAkCommunicationSettings::DefaultDiscoveryBroadcastPort + 2;
}

void UAkSwitchInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_SWITCH
#if AK_ENABLE_COMMUNICATION
	InitializationStructure.CommSettings.bInitSystemLib = false;
#endif
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
	InitializationStructure.DeviceSettings.threadProperties.uStackSize = 1 * 1024 * 1024;
#endif
}
