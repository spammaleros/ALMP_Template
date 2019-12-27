// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkGameplayStatics.h:
=============================================================================*/
#pragma once

#include "AkAudioDevice.h"
#include "AkInclude.h"
#include "AkGameplayTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AkGameplayStatics.generated.h"

// PostEvent functions need to return the PlayingID (uint32), but Blueprints only work with int32.
// Make sure AkPlayingID is always 32 bits, or else we're gonna have a bad time.
static_assert(sizeof(AkPlayingID) == sizeof(int32), "AkPlayingID is not 32 bits anymore. Change return value of PostEvent functions and callback info structures members!");


UCLASS()
class AKAUDIO_API UAkGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Get an AkComponent attached to and following the specified component. 
	 * @param AttachPointName - Optional named point within the AttachComponent to play the sound at.
	 */
	UFUNCTION(BlueprintCallable, Category="Audiokinetic")
	static class UAkComponent * GetAkComponent( class USceneComponent* AttachToComponent, bool& ComponentCreated, FName AttachPointName = NAME_None, FVector Location = FVector(ForceInit), EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset );

	UFUNCTION(BlueprintCallable, Category="Audiokinetic")
	static bool IsEditor();

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static bool IsGame(UObject* WorldContextObject);


	/** Posts a Wwise Event attached to and following the specified component.
	 * @param AkEvent - Wwise Event to post.
	 * @param Actor - actor on which to post the Wwise Event.
	 * @param AttachPointName - Optional named point within the AttachComponent to play the sound at (NOT FUNCTIONAL).
	 * @param bStopWhenAttachedToDestroyed - Specifies whether the sound should stop playing when the owner of the attach to component is destroyed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(DeprecatedFunction), Category="Audiokinetic|Actor", meta = (AdvancedDisplay = "3"))
	static int32 PostEventAttached(	class UAkAudioEvent* AkEvent, 
									class AActor* Actor, 
									FName AttachPointName = NAME_None,
									bool bStopWhenAttachedToDestroyed = false,
									FString EventName = FString(""));
	
	/** Posts a Wwise Event attached to and following the root component of the specified actor.
	 * @param AkEvent - ak event to play.
	 * @param Actor - actor on which to play the event.
	 * @param bStopWhenAttachedToDestroyed - Specifies whether the sound should stop playing when the owner of the attach to component is destroyed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor", meta=(AdvancedDisplay="2", AutoCreateRefTerm = "PostEventCallback,ExternalSources"))
	static int32 PostEvent(	class UAkAudioEvent* AkEvent, 
							class AActor* Actor, 
							UPARAM(meta = (Bitmask, BitmaskEnum = EAkCallbackType)) int32 CallbackMask,
							const FOnAkPostEventCallback& PostEventCallback,
							const TArray<FAkExternalSourceInfo>& ExternalSources,
							bool bStopWhenAttachedToDestroyed = false,
							FString EventName = FString(""));

	AK_DEPRECATED(2019.1.2, "This function is deprecated and will be removed in future releases.")
	static int32 PostEvent( class UAkAudioEvent* AkEvent,
							class AActor* Actor,
							UPARAM(meta = (Bitmask, BitmaskEnum = EAkCallbackType)) int32 CallbackMask,
							const FOnAkPostEventCallback& PostEventCallback,
							bool bStopWhenAttachedToDestroyed = false,
							FString EventName = FString(""));



	/** Posts a Wwise Event attached to and following the root component of the specified actor, and waits for the end of the event to continue execution.
	 * Additional calls made while an event is active are ignored.
	 * @param AkEvent - ak event to play.
	 * @param Actor - actor on which to play the event.
	 * @param bStopWhenAttachedToDestroyed - Specifies whether the sound should stop playing when the owner of the attach to component is destroyed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|Actor", meta = (Latent, LatentInfo = "LatentInfo", AdvancedDisplay = "2", bStopWhenAttachedToDestroyed="false", EventName = "", AutoCreateRefTerm="ExternalSources"))
	static int32 PostAndWaitForEndOfEvent(class UAkAudioEvent* AkEvent,
		class AActor* Actor,
		bool bStopWhenAttachedToDestroyed,
		const TArray<FAkExternalSourceInfo>& ExternalSources,
		FString EventName,
		FLatentActionInfo LatentInfo);

	/** Posts a Wwise Event by name attached to and following the root component of the specified actor.
	 * @param AkEvent - ak event to play.
	 * @param Actor - actor on which to play the event.
	 * @param bStopWhenAttachedToDestroyed - Specifies whether the sound should stop playing when the owner of the attach to component is destroyed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor", meta=(DeprecatedFunction, DeprecationMessage = "Please use the \"Event Name\" field of PostEvent"))
	static void PostEventByName(	const FString& EventName, 
									class AActor* Actor, 
									bool bStopWhenAttachedToDestroyed = false);

	/** Posts a Wwise Event at the specified location. This is a fire and forget sound, created on a temporary Wwise Game Object. Replication is also not handled at this point.
	 * @param AkEvent - Wwise Event to post.
	 * @param Location - Location from which to post the Wwise Event.
	 * @param Orientation - Orientation of the event.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic", meta=(WorldContext="WorldContextObject", AdvancedDisplay = "3"))
	static int32 PostEventAtLocation(class UAkAudioEvent* AkEvent, FVector Location, FRotator Orientation, const FString& EventName, UObject* WorldContextObject );

	/** Posts a Wwise Event by name at the specified location. This is a fire and forget sound, created on a temporary Wwise Game Object. Replication is also not handled at this point.
	 * @param AkEvent - Wwise Event to post.
	 * @param Location - Location from which to post the Wwise Event.
	 * @param Orientation - Orientation of the event.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic", meta=(WorldContext="WorldContextObject", DeprecatedFunction, DeprecationMessage = "Please use the \"Event Name\" field of PostEventAtLocation"))
	static void PostEventAtLocationByName(const FString& EventName, FVector Location, FRotator Orientation, UObject* WorldContextObject );

	/** Execute action on event attached to and following the root component of the specified actor
	 * @param AkEvent - Wwise Event to act upon.
	 * @param ActionType - Which action to do.
	 * @param Actor - Which actor to use.
	 * @param TransitionDuration - Transition duration in milliseconds.
	 * @param FadeCurve - The interpolation curve of the transition.
	 * @param PlayingID - Use the return value of a Post Event to act only on this specific instance of an event.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|Actor")
	static void ExecuteActionOnEvent(class UAkAudioEvent* AkEvent, AkActionOnEventType ActionType, class AActor* Actor, int32 TransitionDuration = 0, EAkCurveInterpolation FadeCurve = EAkCurveInterpolation::Linear, int32 PlayingID = 0);

	/** Execute action on specific playing ID
	 * @param ActionType - Which action to do.
	 * @param PlayingID - Use the return value of a Post Event to act only on this specific instance of an event.
	 * @param TransitionDuration - Transition duration in milliseconds.
	 * @param FadeCurve - The interpolation curve of the transition.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|Actor")
	static void ExecuteActionOnPlayingID(AkActionOnEventType ActionType, int32 PlayingID, int32 TransitionDuration = 0, EAkCurveInterpolation FadeCurve = EAkCurveInterpolation::Linear);

	/** Spawn an AkComponent at a location. Allows, for example, to set a switch on a fire and forget sound.
	 * @param AkEvent - Wwise Event to post.
	 * @param EarlyReflectionsBus - Use the provided auxiliary bus to process early reflections.  If NULL, EarlyReflectionsBusName will be used.
	 * @param Location - Location from which to post the Wwise Event.
	 * @param Orientation - Orientation of the event.
	 * @param AutoPost - Automatically post the event once the AkComponent is created.
	 * @param EarlyReflectionsBusName - Use the provided auxiliary bus to process early reflections.  If empty, no early reflections will be processed.
	 * @param AutoDestroy - Automatically destroy the AkComponent once the event is finished.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic", meta=(WorldContext="WorldContextObject", AdvancedDisplay = "6"))
	static class UAkComponent* SpawnAkComponentAtLocation(UObject* WorldContextObject, class UAkAudioEvent* AkEvent, class UAkAuxBus* EarlyReflectionsBus, FVector Location, FRotator Orientation, bool AutoPost, const FString& EventName, const FString& EarlyReflectionsBusName = FString(""), bool AutoDestroy = true);

	/**
	* Sets the value of a Game Parameter, optionally targetting the root component of a specified actor.
	* @param RTPC - The name of the Game Parameter to set
	* @param Value - The value of the Game Parameter
	* @param InterpolationTimeMs - Duration during which the Game Parameter is interpolated towards Value (in ms)
	* @param Actor - (Optional) Actor on which to set the Game Parameter value
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic")
	static void SetRTPCValue(FName RTPC, float Value, int32 InterpolationTimeMs, class AActor* Actor);

	/**
	* Gets the value of a Game Parameter, optionally targetting the root component of a specified actor.
	* @param RTPC - The name of the Game Parameter to set
	* @param Value - The value of the Game Parameter
	* @param InterpolationTimeMs - Duration during which the Game Parameter is interpolated towards Value (in ms)
	* @param Actor - (Optional) Actor on which to set the Game Parameter value
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic")
	static void GetRTPCValue(FName RTPC, int32 PlayingID, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType, class AActor* Actor = nullptr);

	/**
	 * Set the active State for a given State Group.
	 * @param StateGroup - Name of the State Group to be modified
	 * @param State - Name of the State to be made active
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic")
	static void SetState( FName StateGroup, FName State );
	
	/**
	 * Posts a Trigger, targetting the root component of a specified actor.
	 * @param Trigger - Name of the Trigger
	 * @param Actor - Actor on which to post the Trigger
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor")
	static void PostTrigger( FName Trigger, class AActor* Actor );
	
	/**
	 * Sets the active Switch for a given Switch Group, targetting the root component of a specified actor.
	 * @param SwitchGroup - Name of the Switch Group to be modified
	 * @param SwitchState - Name of the Switch to be made active
	 * @param Actor - Actor on which to set the switch
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor")
	static void SetSwitch( FName SwitchGroup, FName SwitchState, class AActor* Actor );

    /** Sets multiple positions to a single game object.
    *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
    *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
    *  Note: Calling SetMultiplePositions() with only one position is the same as calling SetPosition()
    *  @param GameObjectAkComponent AkComponent of the game object on which to set positions.
    *  @param Positions Array of transforms to apply.
    *  @param MultiPositionType Position type
    *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
    *
    */
    UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
    static void SetMultiplePositions(UAkComponent* GameObjectAkComponent, TArray<FTransform> Positions,
                                     AkMultiPositionType MultiPositionType = AkMultiPositionType::MultiDirections);

    /** Sets multiple positions to a single game object, with flexible assignment of input channels.
    *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
    *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
    *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
    *  @param GameObjectAkComponent AkComponent of the game object on which to set positions.
    *  @param ChannelMasks Array of channel configuration to apply for each position.
    *  @param Positions Array of transforms to apply.
    *  @param MultiPositionType Position type
    *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
    */
    UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
    static void SetMultipleChannelEmitterPositions(UAkComponent* GameObjectAkComponent,
			TArray<AkChannelConfiguration> ChannelMasks,
			TArray<FTransform> Positions,
			AkMultiPositionType MultiPositionType = AkMultiPositionType::MultiDirections
	);

	/** Sets multiple positions to a single game object, with flexible assignment of input channels.
	*  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
	*  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
	*  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
	*  @param GameObjectAkComponent AkComponent of the game object on which to set positions.
	*  @param ChannelMasks Array of channel mask to apply for each position.
	*  @param Positions Array of transforms to apply.
	*  @param MultiPositionType Position type
	*  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static void SetMultipleChannelMaskEmitterPositions(UAkComponent* GameObjectAkComponent,
			TArray<FAkChannelMask> ChannelMasks,
			TArray<FTransform> Positions,
			AkMultiPositionType MultiPositionType = AkMultiPositionType::MultiDirections
	);

	/**
	* Sets UseReverbVolumes flag on a specified actor. Set value to true to use reverb volumes on this component.
	*
	* @param inUseReverbVolumes - Whether to use reverb volumes or not.
	* @param Actor - Actor on which to set the flag
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor")
	static void UseReverbVolumes(bool inUseReverbVolumes, class AActor* Actor);

	/**
	* Sets UseEarlyReflections flag on a specified actor. Set value to true to use calculate and render early reflections on this component.
	*
	* @param Actor - Actor on which to set the flag
	* @param AuxBus Aux bus that contains the AkReflect plugin
	* @param Order Max Order of reflections.
	* @param BusSendGain Send gain for the aux bus.
	* @param MaxPathLength Sound will reflect up to this max distance between emitter and reflective surface.
	* @param EnableSpotReflectors Enable reflections off spot reflectors.
	* @param AuxBusName	Aux bus name that contains the AkReflect plugin
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|Actor", meta = (AdvancedDisplay = "6"))
	static void UseEarlyReflections(class AActor* Actor, 
		class UAkAuxBus* AuxBus,
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
	* @param Actor - Actor on which to set the flag
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|Actor")
	static void SetOutputBusVolume(float BusVolume, class AActor* Actor);

	/**
	* Force channel configuration for the specified bus.
	* This function has unspecified behavior when changing the configuration of a bus that
	* is currently playing.
	* You cannot change the configuration of the master bus.
	*
	* @param BusName				Bus Name
	* @param ChannelConfiguration	Desired channel configuration.
	* @return Always returns AK_Success
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static void SetBusConfig(const FString& BusName, AkChannelConfiguration ChannelConfiguration);

	/**
	*  Set the panning rule of the specified output.
	*  This may be changed anytime once the sound engine is initialized.
	*  @warning This function posts a message through the sound engine's internal message queue, whereas GetPanningRule() queries the current panning rule directly.
	*
	* @param PanRule	Panning rule.
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static void SetPanningRule(PanningRule PanRule);
	
	/**
	 * Gets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
	 * Note that the current version of Wwise only supports positioning on the plane.
	 * The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
	 * Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
	 * Angles must be set in ascending order.
	 * Typical usage:
	 * - float heightAngle;
	 * - TArray<float> speakerAngles;
	 * - GetSpeakerAngles(speakerAngles, heightAngle, AkOutput_Main );
	 * \aknote
	 *  On most platforms, the angle set on the plane consists of 3 angles, to account for 7.1.
	 * - When panning to stereo (speaker mode, see <tt>AK::SoundEngine::SetPanningRule()</tt>), only angle[0] is used, and 3D sounds in the back of the listener are mirrored to the front.
	 * - When panning to 5.1, the front speakers use angle[0], and the surround speakers use (angle[2] - angle[1]) / 2.
	 * \endaknote
	 * \warning Call this function only after the sound engine has been properly initialized.
	 *
	 * @param SpeakerAngles Returned array of loudspeaker pair angles, in degrees relative to azimuth [0,180]. Pass NULL to get the required size of the array.
	 * @param HeightAngle Elevation of the height layer, in degrees relative to the plane [-90,90].
	 * @param DeviceShareset Shareset for which to get the angles. You can pass "" for the main (default) output
	 * @return AK_Success if device exists
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static void GetSpeakerAngles(TArray<float>& SpeakerAngles, float& HeightAngle, const FString& DeviceShareset = "");

	/**
	 * Sets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
	 * Note that the current version of Wwise only supports positioning on the plane.
	 * The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
	 * Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
	 * Angles must be set in ascending order.
	 * Typical usage:
	 * - Initialize the sound engine and/or add secondary output(s).
	 * - Get number of speaker angles and their value into an array using GetSpeakerAngles().
	 * - Modify the angles and call SetSpeakerAngles().
	 * This function posts a message to the audio thread through the command queue, so it is thread safe. However the result may not be immediately read with GetSpeakerAngles().
	 * \warning This function only applies to configurations (or subset of these configurations) that are standard and whose speakers are on the plane (2D).
	 * \sa GetSpeakerAngles()
	 *
	 * @param SpeakerAngles Array of loudspeaker pair angles, in degrees relative to azimuth [0,180]
	 * @param HeightAngle Elevation of the height layer, in degrees relative to the plane [-90,90]
	 * @param DeviceShareset Shareset for which to set the angles on. You can pass "" for the main (default) output
	 * @return AK_Success if successful (device exists and angles are valid), AK_NotCompatible if the channel configuration of the device is not standard (AK_ChannelConfigType_Standard), AK_Fail otherwise.
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static void SetSpeakerAngles(const TArray<float>& SpeakerAngles, float HeightAngle, const FString& DeviceShareset = "");

	/**
	 * Sets the occlusion calculation refresh interval, targetting the root component of a specified actor.
	 * @param RefreshInterval - Value of the wanted refresh interval
	 * @param Actor - Actor on which to set the refresh interval
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor")
	static void SetOcclusionRefreshInterval(float RefreshInterval, class AActor* Actor );

	/**
	 * Stop all sounds for an actor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor")
	static void StopActor(class AActor* Actor);

	/**
	 * Stop all sounds.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic")
	static void StopAll();

	/**
	 * Cancels an Event callback
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic")
	static void CancelEventCallback(const FOnAkPostEventCallback& PostEventCallback);

	/**
	 * Start all Ak ambient sounds.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkAmbientSound", meta=(WorldContext = "WorldContextObject"))
	static void StartAllAmbientSounds(UObject* WorldContextObject);
	
	/**
	 * Stop all Ak ambient sounds.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkAmbientSound", meta=(WorldContext = "WorldContextObject"))
	static void StopAllAmbientSounds(UObject* WorldContextObject);

	
	/**
	 * Clear all loaded banks 
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|SoundBanks")
	static void ClearBanks();

	/* 
	 * Loads a bank.
	 * @param Bank - The bank to load.
	 * 
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|SoundBanks", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", AdvancedDisplay = "1"))
	static void LoadBank(class UAkAudioBank* Bank, const FString& BankName, FLatentActionInfo LatentInfo, UObject* WorldContextObject);
	
	/*
	* Loads a bank asynchronously from Blueprint.
	* @param Bank - The bank to load.
	* @param BankLoadedCallback - Blueprint Delegate to call upon completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|SoundBanks")
	static void LoadBankAsync(class UAkAudioBank* Bank, const FOnAkBankCallback& BankLoadedCallback);

	/* 
	 * Loads a bank by its name.
	 * @param Bank - The bank to load.
	 * 
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|SoundBanks", meta=(DeprecatedFunction, DeprecationMessage = "Please use the \"Bank Name\" field of Load Bank"))
	static void LoadBankByName(const FString& BankName);

	/* 
	 * Loads an array of bank.
	 * @param Banks - An array of banks to load
	 * @param CleanUpBanks - If true, will unload any loaded banks that are not in Banks
	 * 
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|SoundBanks")
	static void LoadBanks(const TArray<UAkAudioBank*>& SoundBanks, bool SynchronizeSoundBanks);
	
	/* 
	 * Loads the init bank.
	 * 
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|SoundBanks")
	static void LoadInitBank();
	
	/**
	 * Unloads a bank.
	 * @param Bank - The bank to unload.
	 */
	UFUNCTION(BlueprintCallable, Category="Audiokinetic|SoundBanks", meta=(WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", AdvancedDisplay="1"))
	static void UnloadBank(class UAkAudioBank* Bank, const FString& BankName, FLatentActionInfo LatentInfo, UObject* WorldContextObject);

	/*
	* Unloads a bank asynchronously from Blueprint.
	* @param Bank - The bank to load.
	* @param BankUnloadedCallback - Blueprint Delegate to call upon completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|SoundBanks")
	static void UnloadBankAsync(class UAkAudioBank* Bank, const FOnAkBankCallback& BankUnloadedCallback);

	/**
	 * Unloads a bank by its name.
	 * @param Bank - The bank to unload.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|SoundBanks", meta = (DeprecatedFunction, DeprecationMessage = "Please use the \"Bank Name\" field of Unload Bank"))
	static void UnloadBankByName(const FString& BankName);

	/**
	 * Starts a Wwise output capture. The output file will be located in the same folder as the SoundBanks.
	 * @param Filename - The name to give to the output file.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Debug")
	static void StartOutputCapture(const FString& Filename);

	/**
	 * Add text marker in output capture file.
	 * @param MarkerText - The name text to put in the marker.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Debug")
	static void AddOutputCaptureMarker(const FString& MarkerText);

	/**
	 * Stops a Wwise output capture. The output file will be located in the same folder as the SoundBanks.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Debug")
	static void StopOutputCapture();
	
	/**
	 * Starts a Wwise profiler capture. The output file will be located in the same folder as the SoundBanks.
	 * @param Filename - The name to give to the output file.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Debug")
	static void StartProfilerCapture(const FString& Filename);

	/**
	 * Stops a Wwise profiler capture. The output file will be located in the same folder as the SoundBanks.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Debug")
	static void StopProfilerCapture();

	/**
	* Allows to globally tweak the occlusion with a multiplicative factor.
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static void SetOcclusionScalingFactor(float ScalingFactor) { OcclusionScalingFactor = ScalingFactor; }

	/**
	* Allows to globally tweak the occlusion with a multiplicative factor.
	*/
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic")
	static float GetOcclusionScalingFactor() { return OcclusionScalingFactor; }

	static bool m_bSoundEngineRecording;

private:

	static float OcclusionScalingFactor;

};