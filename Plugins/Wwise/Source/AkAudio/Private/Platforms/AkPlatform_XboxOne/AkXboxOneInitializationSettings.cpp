// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "Platforms/AkPlatform_XboxOne/AkXboxOneInitializationSettings.h"
#include "AkAudioDevice.h"

#if PLATFORM_XBOXONE
#include <apu.h>

//////////////////////////////////////////////////////////////////////////
// Memory hooks

namespace AK
{
	void* APUAllocHook(size_t in_size, unsigned int in_alignment)
	{
		void* pReturn = nullptr;
		ApuAlloc(&pReturn, NULL, (UINT32)in_size, in_alignment);
		return pReturn;
	}

	void APUFreeHook(void* in_pMemAddress)
	{
		ApuFree(in_pMemAddress);
	}
}
#endif // PLATFORM_XBOXONE

//////////////////////////////////////////////////////////////////////////
// FAkXboxOneAdvancedInitializationSettings

void FAkXboxOneAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if PLATFORM_XBOXONE
	InitializationStructure.PlatformInitSettings.uShapeDefaultPoolSize = ShapeDefaultPoolSize;
	InitializationStructure.PlatformInitSettings.uMaxXMAVoices = MaximumNumberOfXMAVoices;
	InitializationStructure.PlatformInitSettings.bHwCodecLowLatencyMode = UseHardwareCodecLowLatencyMode;
#endif // PLATFORM_XBOXONE
}

//////////////////////////////////////////////////////////////////////////
// FAkXboxOneApuHeapInitializationSettings

void FAkXboxOneApuHeapInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#if PLATFORM_XBOXONE
	// Perform this as early as possible to ensure that no other allocation calls are made before this!
	auto ApuCreateHeapResult = ApuCreateHeap(CachedSize, NonCachedSize);
	if (ApuCreateHeapResult == APU_E_HEAP_ALREADY_ALLOCATED)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("APU heap has already been allocated."));
	}
	else if (ApuCreateHeapResult != S_OK)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("APU heap could not be allocated."));
	}
#endif // PLATFORM_XBOXONE
}


//////////////////////////////////////////////////////////////////////////
// UAkXboxOneInitializationSettings

UAkXboxOneInitializationSettings::UAkXboxOneInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CommonSettings.SamplesPerFrame = 512;

	CommunicationSettings.DiscoveryBroadcastPort = FAkCommunicationSettings::DefaultDiscoveryBroadcastPort;
	CommunicationSettings.CommandPort = FAkCommunicationSettings::DefaultDiscoveryBroadcastPort + 1;
	CommunicationSettings.NotificationPort = FAkCommunicationSettings::DefaultDiscoveryBroadcastPort + 2;
}

void UAkXboxOneInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	ApuHeapSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_XBOXONE && !defined(AK_OPTIMIZED)
	try
	{
		// Make sure networkmanifest.xml is loaded by instantiating a Microsoft.Xbox.Networking object.
		auto secureDeviceAssociationTemplate = Windows::Xbox::Networking::SecureDeviceAssociationTemplate::GetTemplateByName("WwiseDiscovery");
	}
	catch (...)
	{
		UE_LOG(LogAkAudio, Log, TEXT("Could not find Wwise network ports in AppxManifest. Network communication will not be available."));
	}
#endif // PLATFORM_XBOXONE && !defined(AK_OPTIMIZED)
}
