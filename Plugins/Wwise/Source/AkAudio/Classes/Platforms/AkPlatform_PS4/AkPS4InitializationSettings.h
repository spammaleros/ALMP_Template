#pragma once

#include "AkInclude.h"
#include "InitializationSettings/AkInitializationSettings.h"
#include "AkPS4InitializationSettings.generated.h"


USTRUCT()
struct FAkPS4AdvancedInitializationSettings : public FAkAdvancedInitializationSettingsWithMultiCoreRendering
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "ACP batch buffer size used for ATRAC9 decoding."))
	uint32 ACPBatchBufferSize = (90 * 1024);

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Use low latency mode for ATRAC9 (default is false). If true, decoding jobs are submitted at the beginning of the Wwise update and it will be necessary to wait for the result."))
	bool UseHardwareCodecLowLatencyMode;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkPS4InitializationSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommonInitializationSettings CommonSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommunicationSettingsWithSystemInitialization CommunicationSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", AdvancedDisplay)
	FAkPS4AdvancedInitializationSettings AdvancedSettings;

	UFUNCTION()
	void MigrateMultiCoreRendering(bool NewValue)
	{
		AdvancedSettings.EnableMultiCoreRendering = NewValue;
	}
};
