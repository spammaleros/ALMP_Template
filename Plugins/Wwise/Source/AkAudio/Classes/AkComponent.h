// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkComponent.h:
=============================================================================*/

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "AkInclude.h"
#include "AkGameplayTypes.h"
#include "Components/SceneComponent.h"
#include "OcclusionObstructionService/AkComponentOcclusionObstructionService.h"
#include "AkComponent.generated.h"

UENUM(Meta = (Bitflags))
enum class EReflectionFilterBits
{
	Wall,
	Ceiling,
	Floor
};

// PostEvent functions need to return the PlayingID (uint32), but Blueprints only work with int32.
// Make sure AkPlayingID is always 32 bits, or else we're gonna have a bad time.
static_assert(sizeof(AkPlayingID) == sizeof(int32), "AkPlayingID is not 32 bits anymore. Change return value of PostEvent functions!");

struct AkReverbFadeControl
{
public:
	uint32 AuxBusId;
	bool bIsFadingOut;
	void* FadeControlUniqueId; 

private:
	float CurrentControlValue;
	float TargetControlValue;
	float FadeRate;
	float Priority;

public:
	AkReverbFadeControl(const class UAkLateReverbComponent& LateReverbComponent);

	bool Update(float DeltaTime);
	void ForceCurrentToTargetValue() { CurrentControlValue = TargetControlValue; }
	AkAuxSendValue ToAkAuxSendValue() const;

	static bool Prioritize(const AkReverbFadeControl& A, const AkReverbFadeControl& B);
};


/*------------------------------------------------------------------------------------
	UAkComponent
------------------------------------------------------------------------------------*/
UCLASS(ClassGroup=Audiokinetic, BlueprintType, Blueprintable, hidecategories=(Transform,Rendering,Mobility,LOD,Component,Activation), AutoExpandCategories=AkComponent, meta=(BlueprintSpawnableComponent))
class AKAUDIO_API UAkComponent: public USceneComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetUseSpatialAudio, Category = "AkComponent|Spatial Audio")
	bool bUseSpatialAudio = true;

	UFUNCTION(BlueprintSetter)
	void SetUseSpatialAudio(const bool bNewValue);

	/** Wwise Auxiliary Bus for early reflection processing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Reflect")
	class UAkAuxBus * EarlyReflectionAuxBus;

	/** Wwise Auxiliary Bus Name for early reflection processing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Reflect")
	FString EarlyReflectionAuxBusName;

	/** Maximum number of reflections that will be processed when computing indirect paths via the geometric reflections API. Reflection processing grows exponentially with the order of reflections, so this number should be kept low. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Reflect", meta = (ClampMin = "0", ClampMax = "4"))
	int EarlyReflectionOrder;

	/** Send gain (0.f-1.f) that is applied when sending to the bus that has the AkReflect plug-in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Reflect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EarlyReflectionBusSendGain;

	/** A heuristic to stop the computation of reflections. Should be no longer (and possibly shorter for less CPU usage) than the maximum attenuation of the sound emitter. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Reflect", meta = (ClampMin = "0.0"))
	float EarlyReflectionMaxPathLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Occlusion")
	TEnumAsByte<ECollisionChannel> OcclusionCollisionChannel;

	// Note: Reflection fiters are not currently supported on individual polygons in AkSpatialAudioVolume, so it is useless to have it here and therefor hidden from the UI.
	//UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "AkComponent|Spatial Audio", Meta = (Bitmask, BitmaskEnum = "EReflectionFilterBits"))
	int32 ReflectionFilter;

	/** Enable this component's sound to be reflected by spot reflectors. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Reflect")
	uint32 EnableSpotReflectors : 1;

	/** Send gain (0.f-1.f) that is applied when sending to the bus that is associated with the room that the emitter is in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Room", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float roomReverbAuxBusGain;

	/** The maximum number of edges that the sound can diffract around between the emitter and the listener. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Geometric Diffraction", meta = (ClampMin = "0"))
	int diffractionMaxEdges;

	/** The maximum number of paths to the listener that the sound can take around obstacles. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Geometric Diffraction", meta = (ClampMin = "0"))
	int diffractionMaxPaths;

	/** The maximum length that a diffracted sound can travel.  Should be no longer (and possibly shorter for less CPU usage) than the maximum attenuation of the sound emitter. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Geometric Diffraction", meta = (ClampMin = "0.0"))
	float diffractionMaxPathLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	uint32 DrawFirstOrderReflections : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	uint32 DrawSecondOrderReflections : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	uint32 DrawHigherOrderReflections : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	uint32 DrawDiffraction : 1;

	/** Stop sound when owner is destroyed? */
	UPROPERTY()
	bool StopWhenOwnerDestroyed;

	/**
	 * Posts this component's AkAudioEvent to Wwise, using this component as the game object source
	 *
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "PostEventCallback,ExternalSources"))
	int32 PostAssociatedAkEvent(
		UPARAM(meta = (Bitmask, BitmaskEnum = EAkCallbackType)) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback,
		const TArray<FAkExternalSourceInfo>& ExternalSources
	);

	AK_DEPRECATED(2019.1.2, "This function is deprecated and will be removed in future releases.")
	int32 PostAssociatedAkEvent(
		UPARAM(meta = (Bitmask, BitmaskEnum = EAkCallbackType)) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback
	);


	/**
	 * Posts this component's AkAudioEvent to Wwise, using this component as the game object source, and wait until the event is 
	 * done playing to continue execution. Extra calls while the event is playing are ignored.
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent", meta = (AdvancedDisplay = "0", Latent, LatentInfo = "LatentInfo", AutoCreateRefTerm = "ExternalSources"))
	int32 PostAssociatedAkEventAndWaitForEnd(const TArray<FAkExternalSourceInfo>& ExternalSources, FLatentActionInfo LatentInfo);

	AK_DEPRECATED(2019.1.2, "This function is deprecated and will be removed in future releases.")
	int32 PostAssociatedAkEventAndWaitForEnd(FLatentActionInfo LatentInfo);

	/**
	 * Posts an event to Wwise, using this component as the game object source
	 *
	 * @param AkEvent		The event to post
	 * @param CallbackMask	Mask of desired callbacks
	 * @param PostEventCallback	Blueprint Event to execute on callback
	 *
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PostEventCallback,ExternalSources"))
	int32 PostAkEvent(
		class UAkAudioEvent * AkEvent,
		UPARAM(meta = (Bitmask, BitmaskEnum = EAkCallbackType)) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback,
		const TArray<FAkExternalSourceInfo>& ExternalSources,
		const FString& in_EventName
	);

	AK_DEPRECATED(2019.1.2, "This function is deprecated and will be removed in future releases.")
	int32 PostAkEvent(
		class UAkAudioEvent * AkEvent,
		UPARAM(meta = (Bitmask, BitmaskEnum = EAkCallbackType)) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback,
		const FString& in_EventName
	);

	/**
	* Posts an event to Wwise, using this component as the game object source, and wait until the event is
	* done playing to continue execution. Extra calls while the event is playing are ignored.
	*
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent", meta = (AdvancedDisplay = "1", Latent, LatentInfo = "LatentInfo", AutoCreateRefTerm = "ExternalSources"))
	int32 PostAkEventAndWaitForEnd(
		class UAkAudioEvent * AkEvent,
		const FString& in_EventName,
		const TArray<FAkExternalSourceInfo>& ExternalSources,
		FLatentActionInfo LatentInfo
	);

	/**
	 * Posts an event to Wwise using its name, using this component as the game object source
	 *
	 * @param AkEvent		The event to post
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent", meta = (DeprecatedFunction, DeprecationMessage = "Please use the \"Event Name\" field of Post Ak Event"))
	int32 PostAkEventByName( const FString& in_EventName );
	
	AkPlayingID PostAkEventByNameWithDelegate(const FString& in_EventName, int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback, const TArray<FAkExternalSourceInfo>& ExternalSources = TArray<FAkExternalSourceInfo>());
	AkPlayingID PostAkEventByNameWithCallback(const FString& in_EventName, AkUInt32 in_uFlags = 0, AkCallbackFunc in_pfnUserCallback = NULL, void * in_pUserCookie = NULL, const TArray<FAkExternalSourceInfo>& ExternalSources = TArray<FAkExternalSourceInfo>());

	/**
	 * Stops playback using this component as the game object to stop
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void Stop();
	
	/**
	* Sets an RTPC value, using this component as the game object source
	*
	* @param RTPC			The name of the RTPC to set
	* @param Value			The value of the RTPC
	* @param InterpolationTimeMs - Duration during which the RTPC is interpolated towards Value (in ms)
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetRTPCValue(FString RTPC, float Value, int32 InterpolationTimeMs);

	/**
	* Sets an RTPC value, using this component as the game object source
	*
	* @param RTPC			The name of the RTPC to set
	* @param Value			The value of the RTPC
	* @param InterpolationTimeMs - Duration during which the RTPC is interpolated towards Value (in ms)
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void GetRTPCValue(FString RTPC, int32 PlayingID, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType);

	/**
	 * Posts a trigger to wwise, using this component as the game object source
	 *
	 * @param Trigger		The name of the trigger
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void PostTrigger( FString Trigger );
	
	/**
	 * Sets a switch group in wwise, using this component as the game object source
	 *
	 * @param SwitchGroup	The name of the switch group
	 * @param SwitchState	The new state of the switch
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void SetSwitch( FString SwitchGroup, FString SwitchState );

	/**
	 * Sets whether or not to stop sounds when the component's owner is destroyed
	 *
	 * @param bStopWhenOwnerDestroyed	Whether or not to stop sounds when the component's owner is destroyed
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void SetStopWhenOwnerDestroyed( bool bStopWhenOwnerDestroyed );

	/**
	 * Set a game object's listeners
	 *
	 * @param Listeners	The array of components that listen to this component
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void SetListeners( const TArray<UAkComponent*>& Listeners );

	// Reverb volumes functions

	/**
	 * Set UseReverbVolumes flag. Set value to true to use reverb volumes on this component.
	 *
	 * @param inUseReverbVolumes	Whether to use reverb volumes or not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent", meta = (DeprecatedFunction, DeprecationMessage = "Please use the \"UseReverbVolume\" property", ScriptName="DEPRECATED_UseReverbVolumes"))
	void UseReverbVolumes(bool inUseReverbVolumes);

	// Early Reflections

	/**
	* UseEarlyReflections. Enable early reflections for this ak component.
	*
	* @param AuxBus Aux bus that contains the AkReflect plugin
	* @param Order Max Order of reflections.
	* @param BusSendGain Send gain for the aux bus.
	* @param MaxPathLength Sound will reflect up to this max distance between emitter and reflective surface.
	* @param EnableSpotReflectors Enable reflections off spot reflectors.
	* @param AuxBusName	Aux bus name that contains the AkReflect plugin
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent", meta = (AdvancedDisplay = "5"))
	void UseEarlyReflections(class UAkAuxBus* AuxBus,
		int Order = 1,
		float BusSendGain = 1.f,
		float MaxPathLength = 100000.f,
		bool SpotReflectors = false,
		const FString& AuxBusName = FString(""));

	/**
	* Set the output bus volume (direct) to be used for the specified game object.
	* The control value is a number ranging from 0.0f to 1.0f.
	*
	* @param BusVolume - Bus volume to set
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetOutputBusVolume(float BusVolume);


	/** Modifies the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AkComponent")
	float AttenuationScalingFactor;

	/** Sets the attenuation scaling factor, which modifies the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetAttenuationScalingFactor(float Value);

	/** Time interval between occlusion/obstruction checks. Set to 0 to disable occlusion on this component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent")
	float OcclusionRefreshInterval;

	/** Whether to use reverb volumes or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent")
	bool bUseReverbVolumes = true;


	/**
	 * Return the real attenuation radius for this component (AttenuationScalingFactor * AkAudioEvent->MaxAttenuationRadius)
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	float GetAttenuationRadius() const;

	/** Modifies the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AkComponent")
	UAkAudioEvent* AkAudioEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AkComponent")
	FString EventName;

	void UpdateGameObjectPosition();

	void GetAkGameObjectName(FString& Name) const;

	bool IsDefaultListener = false;

#if CPP

	/*------------------------------------------------------------------------------------
		UActorComponent interface.
	------------------------------------------------------------------------------------*/
	/**
	 * Called after component is registered
	 */
	virtual void OnRegister();

	/**
	 * Called after component is unregistered
	 */
	virtual void OnUnregister();

	/**
	 * Clean up
	 */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/**
	 * Clean up after error
	 */
	virtual void ShutdownAfterError();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Begin USceneComponent Interface
	virtual void BeginPlay() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
	// End USceneComponent Interface

	/** Gets all AkLateReverbComponents at the AkComponent's current location, and puts them in a list
	 *
	 * @param Loc					The location of the AkComponent
	 */
	void UpdateAkLateReverbComponentList(FVector Loc);

	/** Gets the current room the AkComponent is in.
	 * 
	 * @param Location			The location of the AkComponent
	 */
	void UpdateSpatialAudioRoom(FVector Location);

	void SetAutoDestroy(bool in_AutoDestroy) { bAutoDestroy = in_AutoDestroy; }

	bool UseDefaultListeners() const { return bUseDefaultListeners; }

	bool HasActiveEvents() const;

	void OnListenerUnregistered(UAkComponent* in_pListener)
	{
		Listeners.Remove(in_pListener);
	}

	void OnDefaultListenerAdded(UAkComponent* in_pListener)
	{
		check(bUseDefaultListeners);
		Listeners.Add(in_pListener);
	}

	const TSet<UAkComponent*>& GetEmitters();

	AkGameObjectID GetAkGameObjectID() const;
	static UAkComponent* GetAkComponent(AkGameObjectID GameObjectID);
	bool AllowAudioPlayback() const;

	bool VerifyEventName(const FString& in_EventName) const;

	AkRoomID GetSpatialAudioRoom() const;

	const FTransform& GetTransform() const;

	void UpdateOcclusionObstruction() { ObstructionService.UpdateObstructionOcclusion(Listeners, GetPosition(), GetOwner(), GetSpatialAudioRoom(), OcclusionCollisionChannel, OcclusionRefreshInterval); }

	FVector GetPosition() const;

private:
	/**
	 * Register the component with Wwise
	 */
	void RegisterGameObject();

	/**
	 * Unregister the component from Wwise
	 */
	void UnregisterGameObject();

	/*
	* Called after registering the component with Wwise
	*/
	virtual void PostRegisterGameObject();

	/*
	* Called after unregistering the component from Wwise
	*/
	virtual void PostUnregisterGameObject();

	bool IsRegisteredWithWwise = false;

	// Reverb Volume features ---------------------------------------------------------------------

	/** Apply the current list of AkReverbVolumes 
	 *
	 * @param DeltaTime		The given time increment since last fade computation
	 */
	void ApplyAkReverbVolumeList(float DeltaTime);

	AkComponentOcclusionObstructionService ObstructionService;

	/** Array of the active AkReverbVolumes at the AkComponent's location */
	TArray<AkReverbFadeControl> ReverbFadeControls;

	/** Aux Send values sent to the SoundEngine in the previous frame */
	TArray<AkAuxSendValue> CurrentAuxSendValues;

	/** Do we need to refresh Aux Send values? */
	bool NeedToUpdateAuxSends(const TArray<AkAuxSendValue>& NewValues);

	/** Room the AkComponent is currently in. nullptr if none */
	class UAkRoomComponent* CurrentRoom;

	/** Whether to automatically destroy the component when the event is finished */
	bool bAutoDestroy;

	/** Whether an event was posted on the component. Never reset to false. */
	bool bStarted;

	/** Previous known position. Used to avoid Spamming SetPOsition on a listener */
	AkSoundPosition CurrentSoundPosition;
	bool HasMoved();

#endif

#if WITH_EDITORONLY_DATA
	/** Utility function that updates which texture is displayed on the sprite dependent on the properties of the Audio Component. */
	void UpdateSpriteTexture();
#endif

	bool bUseDefaultListeners;
	TSet<UAkComponent*> Listeners;

	//NOTE: This set of emitters is only valid if this UAkComopnent is a listener, and it it is not a default listener. See GetEmitters().
	TSet<UAkComponent*> Emitters;

	void CheckEmitterListenerConsistancy();

	void DebugDrawReflections() const;
	void _DebugDrawReflections(const AkVector& akEmitterPos, const AkVector& akListenerPos, const AkReflectionPathInfo* paths, AkUInt32 uNumPaths) const;

	void DebugDrawDiffraction() const;
	void _DebugDrawDiffraction(const AkVector& akEmitterPos, const AkVector& akListenerPos, const AkDiffractionPathInfo* paths, AkUInt32 uNumPaths) const;
};
