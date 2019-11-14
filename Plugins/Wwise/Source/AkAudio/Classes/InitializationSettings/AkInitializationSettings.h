#pragma once

#include "Engine/EngineTypes.h"
#include "AkInclude.h"
#include "AkInitializationSettings.generated.h"

UENUM()
enum class EAkPanningRule
{
	Speakers = AkPanningRule::AkPanningRule_Speakers,
	Headphones = AkPanningRule::AkPanningRule_Headphones,
};

UENUM(Meta = (Bitmask))
enum class EAkDiffractionFlags : uint32
{
	UseBuiltInParam = 0,
	UseObstruction = 1,
	CalcEmitterVirtualPosition = 3,
};

static_assert((1 << (uint32)EAkDiffractionFlags::UseBuiltInParam) == AkDiffractionFlags::DiffractionFlags_UseBuiltInParam, "Review constants defined in \"include\\AK\\SpatialAudio\\Common\\AkSpatialAudio.h\"");
static_assert((1 << (uint32)EAkDiffractionFlags::UseObstruction) == AkDiffractionFlags::DiffractionFlags_UseObstruction, "Review constants defined in \"include\\AK\\SpatialAudio\\Common\\AkSpatialAudio.h\"");
static_assert((1 << (uint32)EAkDiffractionFlags::CalcEmitterVirtualPosition) == AkDiffractionFlags::DiffractionFlags_CalcEmitterVirtualPosition, "Review constants defined in \"include\\AK\\SpatialAudio\\Common\\AkSpatialAudio.h\"");

UENUM()
enum class EAkChannelConfigType
{
	Anonymous = AkChannelConfigType::AK_ChannelConfigType_Anonymous,
	Standard = AkChannelConfigType::AK_ChannelConfigType_Standard,
	Ambisonic = AkChannelConfigType::AK_ChannelConfigType_Ambisonic,
};

UENUM(Meta = (Bitmask))
enum class EAkChannelMask : uint32
{
	FrontLeft,
	FrontRight,
	FrontCenter,
	LowFrequency,
	BackLeft,
	BackRight,
	BackCenter = BackRight + 3,
	SideLeft,
	SideRight,

	Top,
	HeightFrontLeft,
	HeightFrontCenter,
	HeightFrontRight,
	HeightBackLeft,
	HeightBackCenter,
	HeightBackRight,
};

static_assert((1 << (uint32)EAkChannelMask::FrontLeft) == AK_SPEAKER_FRONT_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::FrontRight) == AK_SPEAKER_FRONT_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::FrontCenter) == AK_SPEAKER_FRONT_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::LowFrequency) == AK_SPEAKER_LOW_FREQUENCY, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::BackLeft) == AK_SPEAKER_BACK_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::BackRight) == AK_SPEAKER_BACK_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::BackCenter) == AK_SPEAKER_BACK_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::SideLeft) == AK_SPEAKER_SIDE_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::SideRight) == AK_SPEAKER_SIDE_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::Top) == AK_SPEAKER_TOP, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightFrontLeft) == AK_SPEAKER_HEIGHT_FRONT_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightFrontCenter) == AK_SPEAKER_HEIGHT_FRONT_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightFrontRight) == AK_SPEAKER_HEIGHT_FRONT_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightBackLeft) == AK_SPEAKER_HEIGHT_BACK_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightBackCenter) == AK_SPEAKER_HEIGHT_BACK_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightBackRight) == AK_SPEAKER_HEIGHT_BACK_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");

struct FAkInitializationStructure
{
	FAkInitializationStructure();

	AkMemSettings MemSettings;
	AkStreamMgrSettings StreamManagerSettings;
	AkDeviceSettings DeviceSettings;
	AkInitSettings InitSettings;
	AkPlatformInitSettings PlatformInitSettings;
	AkMusicSettings MusicSettings;
	AkSpatialAudioInitSettings SpatialAudioInitSettings;
#ifndef AK_OPTIMIZED
	AkCommSettings CommSettings;
#endif
};


USTRUCT()
struct FAkMainOutputSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "The name of a custom audio device to be used. Custom audio devices are defined in the Audio Device Shareset section of the Wwise project. Leave this empty to output normally through the default audio device."))
	FString AudioDeviceShareset;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "Device specific identifier, when multiple devices of the same type are possible. If only one device is possible, leave to 0."))
	uint32 DeviceID = AK_INVALID_UNIQUE_ID;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "Rule for 3D panning of signals routed to a stereo bus. In \"Speakers\" mode, the angle of the front loudspeakers is used. In \"Headphones\" mode, the speaker angles are superseded with constant power panning between two virtual microphones spaced 180 degrees apart."))
	EAkPanningRule PanningRule = EAkPanningRule::Speakers;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "A code that completes the identification of channels by uChannelMask. Anonymous: Channel mask == 0 and channels; Standard: Channels must be identified with standard defines in AkSpeakerConfigs; Ambisonic: Channel mask == 0 and channels follow standard ambisonic order."))
	EAkChannelConfigType ChannelConfigType = EAkChannelConfigType::Anonymous;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (Bitmask, BitmaskEnum = EAkChannelMask, ToolTip = "A bit field, whose channel identifiers depend on AkChannelConfigType (up to 20)."))
	uint32 ChannelMask = 0;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "The number of channels, identified (deduced from channel mask) or anonymous (set directly)."))
	uint32 NumberOfChannels = 0;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkSpatialAudioSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Desired spatial audio memory pool size."))
	uint32 SpatialAudioPoolSize = 4 * 1024 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Maximum number of portals that sound can propagate through.", ClampMin = "0", ClampMax = "8"))
	uint32 MaxSoundPropagationDepth = AK_MAX_SOUND_PROPAGATION_DEPTH;
	static_assert(AK_MAX_SOUND_PROPAGATION_DEPTH == 8, "AK_MAX_SOUND_PROPAGATION_DEPTH has changed values. Ensure that the limits of MaxSoundPropagationDepth are equally reflected.");

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (Bitmask, BitmaskEnum = EAkDiffractionFlags, ToolTip = "Determines whether diffraction values for sound passing through portals will be calculated, and how to apply those calculations to Wwise parameters."))
	uint32 DiffractionFlags = AkDiffractionFlags::DefaultDiffractionFlags;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Distance (in game units) that an emitter or listener has to move to trigger a recalculation of reflections/diffraction. Larger values can reduce the CPU load at the cost of reduced accuracy.", ClampMin = "0"))
	float MovementThreshold = 10.f;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkAdvancedSpatialAudioSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Advanced Spatial Audio Settings", AdvancedDisplay, meta = (ToolTip = "Multiplier that is applied to the distance attenuation of diffracted sounds (sounds that are in the 'shadow region') to simulate the phenomenon where by diffracted sound waves decay faster than incident sound waves."))
	float DiffractionShadowAttenuationFactor = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Advanced Spatial Audio Settings", AdvancedDisplay, meta = (ToolTip = "Interpolation angle, in degrees, over which the \"Diffraction Shadow Attenuation Factor\" is applied."))
	float DiffractionShadowDegrees = 30.0f;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkCommunicationSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "Size of the communication pool."))
	uint32 PoolSize = 256 * 1024;

	enum { DefaultDiscoveryBroadcastPort = 24024, };
	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The port where the authoring application broadcasts \"Game Discovery\" requests to discover games running on the network. Default value: 24024. (Cannot be set to 0)."))
	uint16 DiscoveryBroadcastPort = DefaultDiscoveryBroadcastPort;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The \"command\" channel port. Set to 0 to request a dynamic/ephemeral port."))
	uint16 CommandPort = 0;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The \"notification\" channel port. Set to 0 to request a dynamic/ephemeral port."))
	uint16 NotificationPort = 0;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The name used to identify this game within the authoring application. Leave empty to use FApp::GetProjectName()."))
	FString NetworkName;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;

protected:
	FString GetCommsNetworkName() const;
};


USTRUCT()
struct FAkCommunicationSettingsWithSystemInitialization : public FAkCommunicationSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Config, Category = "Ak Initialization Settings|Communication Settings", EditAnywhere, meta = (ToolTip = "Indicates whether the communication system should be initialized. Some consoles have critical requirements for initialization of their communications system. Set to false only if your game already uses sockets before sound engine initialization."))
	bool InitializeSystemComms = true;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkCommonInitializationSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of memory pools. A memory pool is required for each loaded bank."))
	uint32 MaximumNumberOfMemoryPools = 256;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of automation paths for positioning sounds."))
	uint32 MaximumNumberOfPositioningPaths = 255;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of the default memory pool."))
	uint32 DefaultPoolSize = 128 * 1024 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "The percentage of occupied memory where the sound engine should enter in Low memory Mode.", ClampMin = "0.0", ClampMax = "1.0"))
	float MemoryCutoffThreshold = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of the command queue."))
	uint32 CommandQueueSize = 256 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of samples per audio frame (256, 512, 1024, or 2048)."))
	uint32 SamplesPerFrame = 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Platform-independent initialization settings of output devices."))
	FAkMainOutputSettings MainOutputSettings;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Multiplication factor for all streaming look-ahead heuristic values.", ClampMin = "0.0", ClampMax = "1.0"))
	float StreamingLookAheadRatio = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of memory pool for small objects of Stream Manager. Small objects are the Stream Manager instance, devices, stream objects, user stream names, pending transfers, buffer records, pending open commands, and so on. Ideally, this pool should never run out of memory, because it may cause undesired I/O transfer cancellation, and even major CPU spikes. I/O memory should be bound by the size of each device's I/O pool instead."))
	uint32 StreamManagerPoolSize = 64 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Lower Engine default memory pool size."))
	uint32 LowerEnginePoolSize = 16 * 1024 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "The percentage of occupied memory where the sound engine should enter in Low memory mode.", ClampMin = "0.0", ClampMax = "1.0"))
	float LowerEngineMemoryCutoffThreshold = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of refill buffers in voice buffer. Set to 2 for double-buffered, defaults to 4."))
	uint16 NumberOfRefillsInVoice = 4;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings")
	FAkSpatialAudioSettings SpatialAudioSettings;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkCommonInitializationSettingsWithSampleRate : public FAkCommonInitializationSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Config, EditAnywhere, Category = "Common Settings", meta = (ToolTip = "Sampling Rate. Default is 48000 Hz. Use 24000hz for low quality. Any positive reasonable sample rate is supported; however, be careful setting a custom value. Using an odd or really low sample rate may cause the sound engine to malfunction."))
	uint32 SampleRate = 48000;
};


USTRUCT()
struct FAkAdvancedInitializationSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of memory pool for I/O (for automatic streams). It is passed directly to AK::MemoryMgr::CreatePool(), after having been rounded down to a multiple of uGranularity."))
	uint32 IO_MemorySize = 2 * 1024 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Targeted automatic stream buffer length (ms). When a stream reaches that buffering, it stops being scheduled for I/O except if the scheduler is idle."))
	float TargetAutoStreamBufferLength = 380.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "If true the device attempts to reuse IO buffers that have already been streamed from disk. This is particularly useful when streaming small looping sounds. The drawback is a small CPU hit when allocating memory, and a slightly larger memory footprint in the StreamManager pool."))
	bool UseStreamCache = false;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of bytes that can be \"pinned\" using AK::SoundEngine::PinEventInStreamCache() or AK::IAkStreamMgr::PinFileInCache()."))
	uint32 MaximumPinnedBytesInCache = (uint32)-1;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Memory pool where data allocated by AK::SoundEngine::PrepareEvent() and AK::SoundEngine::PrepareGameSyncs() will be done."))
	int32 PrepareEventMemoryPoolID = AK_INVALID_POOL_ID;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Set to true to enable AK::SoundEngine::PrepareGameSync usage."))
	bool EnableGameSyncPreparation = false;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of quanta ahead when continuous containers should instantiate a new voice before which next sounds should start playing. This look-ahead time allows I/O to occur, and is especially useful to reduce the latency of continuous containers with trigger rate or sample-accurate transitions."))
	uint32 ContinuousPlaybackLookAhead = 1;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of the monitoring pool. This parameter is not used in Release build."))
	uint32 MonitorPoolSize = 256 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of the monitoring queue pool. This parameter is not used in Release build."))
	uint32 MonitorQueuePoolSize = 64 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Amount of time to wait for hardware devices to trigger an audio interrupt. If there is no interrupt after that time, the sound engine will revert to silent mode and continue operating until the hardware finally comes back."))
	uint32 MaximumHardwareTimeoutMs = 1000;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings")
	FAkAdvancedSpatialAudioSettings SpatialAudioSettings;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};

USTRUCT()
struct FAkAdvancedInitializationSettingsWithMultiCoreRendering : public FAkAdvancedInitializationSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Allow to distribute SoundEngine processing tasks across multiple threads. Requires Editor restart."))
	bool EnableMultiCoreRendering = false;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};

class CAkUnrealIOHookDeferred;

namespace FAkSoundEngineInitialization
{
	bool Initialize(CAkUnrealIOHookDeferred* IOHookDeferred);
	void Finalize();
}
