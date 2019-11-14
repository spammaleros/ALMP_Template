// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "Platforms/AkPlatform_PS4/AkPS4InitializationSettings.h"
#include "AkAudioDevice.h"

//////////////////////////////////////////////////////////////////////////
// UAkPS4InitializationSettings

void FAkPS4AdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if PLATFORM_PS4
	InitializationStructure.PlatformInitSettings.uLEngineAcpBatchBufferSize = ACPBatchBufferSize;
	InitializationStructure.PlatformInitSettings.bHwCodecLowLatencyMode = UseHardwareCodecLowLatencyMode;
#endif
}


//////////////////////////////////////////////////////////////////////////
// UAkPS4InitializationSettings

UAkPS4InitializationSettings::UAkPS4InitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CommonSettings.SamplesPerFrame = 512;
}

void UAkPS4InitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);
}
