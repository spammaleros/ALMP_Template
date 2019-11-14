// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "InitializationSettings/AkInitializationSettings.h"
#include "Platforms/AkUEPlatform.h"

#include "AkUnrealIOHookDeferred.h"
#include "Async/ParallelFor.h"

namespace AkInitializationSettings_Helpers
{
	void ParallelFor(void* data, AkUInt32 beginIndex, AkUInt32 endIndex, AkUInt32 /*tileSize*/, AkParallelForFunc func, void* userData, const char* in_szDebugName)
	{
		check(func);
		check(endIndex >= beginIndex);

		if (func != nullptr && endIndex - beginIndex > 0)
		{
			::ParallelFor(endIndex - beginIndex, [data, beginIndex, func, userData](int32 Index)
			{
				check(data);

				AkTaskContext ctx; // Unused in the SoundEngine right now.
				ctx.uIdxThread = 0;
				func(data, beginIndex + Index, beginIndex + Index + 1, ctx, userData);
			});
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// FAkInitializationStructure

FAkInitializationStructure::FAkInitializationStructure()
{
	AK::StreamMgr::GetDefaultSettings(StreamManagerSettings);

	AK::StreamMgr::GetDefaultDeviceSettings(DeviceSettings);
	DeviceSettings.uGranularity = AK_UNREAL_IO_GRANULARITY;
	DeviceSettings.uSchedulerTypeFlags = AK_SCHEDULER_DEFERRED_LINED_UP;
	DeviceSettings.uMaxConcurrentIO = AK_UNREAL_MAX_CONCURRENT_IO;

	AK::SoundEngine::GetDefaultInitSettings(InitSettings);
	InitSettings.eFloorPlane = AkFloorPlane_XY;

	AK::SoundEngine::GetDefaultPlatformInitSettings(PlatformInitSettings);

	AK::MusicEngine::GetDefaultInitSettings(MusicSettings);

#if AK_ENABLE_COMMUNICATION
	AK::Comm::GetDefaultInitSettings(CommSettings);
#endif
}


//////////////////////////////////////////////////////////////////////////
// FAkMainOutputSettings

void FAkMainOutputSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& OutputSettings = InitializationStructure.InitSettings.settingsMainOutput;

	auto sharesetID = !AudioDeviceShareset.IsEmpty() ? AK::SoundEngine::GetIDFromString(TCHAR_TO_ANSI(*AudioDeviceShareset)) : AK_INVALID_UNIQUE_ID;
	OutputSettings.audioDeviceShareset = sharesetID;

	switch (ChannelConfigType)
	{
	case EAkChannelConfigType::Anonymous:
		OutputSettings.channelConfig.SetAnonymous(NumberOfChannels);
		break;

	case EAkChannelConfigType::Standard:
		OutputSettings.channelConfig.SetStandard(ChannelMask);
		break;

	case EAkChannelConfigType::Ambisonic:
		OutputSettings.channelConfig.SetAmbisonic(NumberOfChannels);
		break;
	}

	OutputSettings.ePanningRule = (AkPanningRule)PanningRule;
	OutputSettings.idDevice = DeviceID;
}


//////////////////////////////////////////////////////////////////////////
// FAkSpatialAudioSettings

void FAkSpatialAudioSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& SpatialAudioInitSettings = InitializationStructure.SpatialAudioInitSettings;
	SpatialAudioInitSettings.uPoolSize = SpatialAudioPoolSize;
	SpatialAudioInitSettings.uMaxSoundPropagationDepth = MaxSoundPropagationDepth;
	SpatialAudioInitSettings.uDiffractionFlags = DiffractionFlags;
	SpatialAudioInitSettings.fMovementThreshold = MovementThreshold;
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedSpatialAudioSettings

void FAkAdvancedSpatialAudioSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& SpatialAudioInitSettings = InitializationStructure.SpatialAudioInitSettings;
	SpatialAudioInitSettings.fDiffractionShadowAttenFactor = DiffractionShadowAttenuationFactor;
	SpatialAudioInitSettings.fDiffractionShadowDegrees = DiffractionShadowDegrees;
}


//////////////////////////////////////////////////////////////////////////
// FAkCommunicationSettings

void FAkCommunicationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#ifndef AK_OPTIMIZED
	auto& CommSettings = InitializationStructure.CommSettings;
	CommSettings.uPoolSize = PoolSize;
	CommSettings.ports.uDiscoveryBroadcast = DiscoveryBroadcastPort;
	CommSettings.ports.uCommand = CommandPort;
	CommSettings.ports.uNotification = NotificationPort;

	const FString GameName = GetCommsNetworkName();
	FCStringAnsi::Strcpy(CommSettings.szAppNetworkName, AK_COMM_SETTINGS_MAX_STRING_SIZE, TCHAR_TO_ANSI(*GameName));
#endif // AK_OPTIMIZED
}

FString FAkCommunicationSettings::GetCommsNetworkName() const
{
	FString CommsNetworkName = NetworkName;

	if (CommsNetworkName.IsEmpty() && FApp::HasProjectName())
	{
		CommsNetworkName = FApp::GetProjectName();
	}

#if WITH_EDITORONLY_DATA
	if (!CommsNetworkName.IsEmpty() && !IsRunningGame())
	{
		CommsNetworkName += TEXT(" (Editor)");
	}
#endif

	return CommsNetworkName;
}


//////////////////////////////////////////////////////////////////////////
// FAkCommunicationSettingsWithSystemInitialization

void FAkCommunicationSettingsWithSystemInitialization::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if AK_ENABLE_COMMUNICATION
	InitializationStructure.CommSettings.bInitSystemLib = InitializeSystemComms;
#endif
}


//////////////////////////////////////////////////////////////////////////
// FAkCommonInitializationSettings

void FAkCommonInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	InitializationStructure.MemSettings.uMaxNumPools = MaximumNumberOfMemoryPools;

	InitializationStructure.StreamManagerSettings.uMemorySize = StreamManagerPoolSize;

	auto& InitSettings = InitializationStructure.InitSettings;
	InitSettings.uMaxNumPaths = MaximumNumberOfPositioningPaths;
	InitSettings.uDefaultPoolSize = DefaultPoolSize;
	InitSettings.fDefaultPoolRatioThreshold = MemoryCutoffThreshold;
	InitSettings.uCommandQueueSize = CommandQueueSize;
	InitSettings.uNumSamplesPerFrame = SamplesPerFrame;
	InitSettings.szPluginDLLPath = nullptr;

	MainOutputSettings.FillInitializationStructure(InitializationStructure);

	auto& PlatformInitSettings = InitializationStructure.PlatformInitSettings;
	PlatformInitSettings.uLEngineDefaultPoolSize = LowerEnginePoolSize;
	PlatformInitSettings.fLEngineDefaultPoolRatioThreshold = LowerEngineMemoryCutoffThreshold;
	PlatformInitSettings.uNumRefillsInVoice = NumberOfRefillsInVoice;

	SpatialAudioSettings.FillInitializationStructure(InitializationStructure);

	InitializationStructure.MusicSettings.fStreamingLookAheadRatio = StreamingLookAheadRatio;
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedInitializationSettings

void FAkAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& DeviceSettings = InitializationStructure.DeviceSettings;
	DeviceSettings.uIOMemorySize = IO_MemorySize;
	DeviceSettings.fTargetAutoStmBufferLength = TargetAutoStreamBufferLength;
	DeviceSettings.bUseStreamCache = UseStreamCache;
	DeviceSettings.uMaxCachePinnedBytes = MaximumPinnedBytesInCache;

	auto& InitSettings = InitializationStructure.InitSettings;
	InitSettings.uPrepareEventMemoryPoolID = PrepareEventMemoryPoolID;
	InitSettings.bEnableGameSyncPreparation = EnableGameSyncPreparation;
	InitSettings.uContinuousPlaybackLookAhead = ContinuousPlaybackLookAhead;
	InitSettings.uMonitorPoolSize = MonitorPoolSize;
	InitSettings.uMonitorQueuePoolSize = MonitorQueuePoolSize;
	InitSettings.uMaxHardwareTimeoutMs = MaximumHardwareTimeoutMs;

	SpatialAudioSettings.FillInitializationStructure(InitializationStructure);
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedInitializationSettingsWithMultiCoreRendering

void FAkAdvancedInitializationSettingsWithMultiCoreRendering::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

	if (EnableMultiCoreRendering)
	{
		check(FTaskGraphInterface::Get().IsRunning());
		check(FPlatformProcess::SupportsMultithreading());
		check(ENamedThreads::bHasHighPriorityThreads);

		auto& InitSettings = InitializationStructure.InitSettings;
		InitSettings.taskSchedulerDesc.fcnParallelFor = AkInitializationSettings_Helpers::ParallelFor;
		InitSettings.taskSchedulerDesc.uNumSchedulerWorkerThreads = FTaskGraphInterface::Get().GetNumWorkerThreads();
	}
}

namespace FAkSoundEngineInitialization
{
	bool Initialize(CAkUnrealIOHookDeferred* IOHookDeferred)
	{
		if (IOHookDeferred == nullptr)
			return false;

		const UAkInitializationSettings* InitializationSettings = FAkPlatform::GetInitializationSettings();
		if (InitializationSettings == nullptr)
			return false;

		FAkInitializationStructure InitializationStructure;
		InitializationSettings->FillInitializationStructure(InitializationStructure);

		if (AK::MemoryMgr::Init(&InitializationStructure.MemSettings) != AK_Success)
			return false;

		if (!AK::StreamMgr::Create(InitializationStructure.StreamManagerSettings))
			return false;

		if (!IOHookDeferred->Init(InitializationStructure.DeviceSettings))
			return false;

		if (AK::SoundEngine::Init(&InitializationStructure.InitSettings, &InitializationStructure.PlatformInitSettings) != AK_Success)
			return false;

		if (AK::MusicEngine::Init(&InitializationStructure.MusicSettings) != AK_Success)
			return false;

		if (AK::SpatialAudio::Init(InitializationStructure.SpatialAudioInitSettings) != AK_Success)
			return false;

#if AK_ENABLE_COMMUNICATION
		if (AK::Comm::Init(InitializationStructure.CommSettings) != AK_Success)
		{
			UE_LOG(LogInit, Warning, TEXT("Could not initialize communication."));
		}
#endif

		return true;
	}

	void Finalize()
	{
#if AK_ENABLE_COMMUNICATION
		AK::Comm::Term();
#endif

		AK::MusicEngine::Term();

		AK::SoundEngine::Term();

		if (auto* StreamManager = AK::IAkStreamMgr::Get())
			StreamManager->Destroy();

		AK::MemoryMgr::Term();
	}
}
