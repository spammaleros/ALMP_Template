// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkAudioDevice.h: Audiokinetic audio interface object.
=============================================================================*/

#pragma once

/*------------------------------------------------------------------------------------
	AkAudioDevice system headers
------------------------------------------------------------------------------------*/

#include "AkInclude.h"
#include <AK/SoundEngine/Common/AkTypes.h>
#include "AkBankManager.h"
#include "AkGameplayTypes.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"

#if WITH_EDITORONLY_DATA
#include "EditorViewportClient.h"
#endif

#define GET_AK_EVENT_NAME(AkEvent, EventName) ((AkEvent) ? ((AkEvent)->GetName()) : (EventName))


DECLARE_LOG_CATEGORY_EXTERN(LogAkAudio, Log, All);
DECLARE_EVENT(FAkAudioDevice, SoundbanksLoaded);
/*------------------------------------------------------------------------------------
	Dependencies, helpers & forward declarations.
------------------------------------------------------------------------------------*/

class UAkComponent;
class UAkLateReverbComponent;
class FAkComponentCallbackManager;
class CAkUnrealIOHookDeferred;
class AkFileCustomParamPolicy;
class AAkAcousticPortal;
class CAkDiskPackage;
class AkCallbackInfoPool;

template <class T_LLIOHOOK_FILELOC, class T_PACKAGE, class U_CUSTOMPARAM_POLICY>
class CAkFilePackageLowLevelIO;

typedef TSet<UAkComponent*> UAkComponentSet;

#define DUMMY_GAMEOBJ ((AkGameObjectID)0x2)
#define SOUNDATLOCATION_GAMEOBJ ((AkGameObjectID)0x3)

#ifdef AK_SOUNDFRAME
namespace AK {
	namespace SoundFrame {
		class IClient;
		class ISoundFrame;
	}
}
#endif

/** Define hashing for AkGameObjectID. */
template<typename ValueType, bool bInAllowDuplicateKeys>
struct AkGameObjectIdKeyFuncs : TDefaultMapKeyFuncs<AkGameObjectID, ValueType, bInAllowDuplicateKeys>
{
	static FORCEINLINE uint32 GetKeyHash(AkGameObjectID Key)
	{
		if (sizeof(Key) <= 4)
		{
			return (uint32)Key;
		}
		else
		{
			// Copied from GetTypeHash( const uint64 A ) found in ...\Engine\Source\Runtime\Core\Public\Templates\TypeHash.h
			return (uint32)Key + ((uint32)(Key >> 32) * 23);
		}
	}
};


struct AKAUDIO_API FAkAudioDeviceDelegates
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAkGlobalCallback, AK::IAkGlobalPluginContext*, AkGlobalCallbackLocation);
};


/*------------------------------------------------------------------------------------
	Audiokinetic audio device.
------------------------------------------------------------------------------------*/
 
class AKAUDIO_API FAkAudioDevice
#ifdef AK_SOUNDFRAME
	: public AK::SoundFrame::IClient
#endif
{
public:
	//virtual bool Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	//{
	//	return true;
	//}

	virtual ~FAkAudioDevice() {}

	/**
	 * Initializes the audio device and creates sources.
	 *
	 * @return true if initialization was successful, false otherwise
	 */
	virtual bool Init( void );

	/**
	 * Update the audio device and calculates the cached inverse transform later
	 * on used for spatialization.
	 */
	virtual bool Update( float DeltaTime );
	
	/**
	 * Tears down audio device by stopping all sounds, removing all buffers, 
	 * destroying all sources, ... Called by both Destroy and ShutdownAfterError
	 * to perform the actual tear down.
	 */
	virtual void Teardown();

	/**
	 * Stops all game sounds (and possibly UI) sounds
	 *
	 * @param bShouldStopUISounds If true, this function will stop UI sounds as well
	 */
	virtual void StopAllSounds( bool bShouldStopUISounds = false );

	/**
	 * Stop all audio associated with a scene
	 *
	 * @param SceneToFlush		Interface of the scene to flush
	 */
	void Flush(UWorld* WorldToFlush);

	/**
	 * Clears all loaded soundbanks
	 *
	 * @return Result from ak sound engine 
	 */
	AKRESULT ClearBanks();

	/**
	 * Load a soundbank
	 *
	 * @param in_Bank			The bank to load
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID		Returned bank ID
	 * @return Result from ak sound engine 
	 */
	AKRESULT LoadBank(
		class UAkAudioBank *	in_Bank,
		AkMemPoolId		in_memPoolId,
		AkBankID &      out_bankID
		);

	/**
	 * Load a soundbank by name
	 *
	 * @param in_BankName			The name of the bank to load
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID		Returned bank ID
	 * @return Result from ak sound engine 
	 */
	AKRESULT LoadBank(
		const FString&	in_BankName,
		AkMemPoolId		in_memPoolId,
		AkBankID &      out_bankID
		);

	/**
	 * Load a soundbank asynchronously
	 *
	 * @param in_Bank			The bank to load
	 * @param in_pfnBankCallback Callback function
	 * @param in_pCookie		Callback cookie (reserved to user, passed to the callback function)
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID		Returned bank ID
	 * @return Result from ak sound engine 
	 */
	AKRESULT LoadBank(
        class UAkAudioBank *      in_Bank,
		AkBankCallbackFunc  in_pfnBankCallback,
		void *              in_pCookie,
        AkMemPoolId         in_memPoolId,
		AkBankID &          out_bankID
        );
		
	/**
	 * Load a soundbank asynchronously, flagging completion in the latent action
	 *
	 * @param in_Bank			The bank to load
	 * @param LoadBankLatentAction Blueprint Latent action to flag completion
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT LoadBank(
		class UAkAudioBank *     in_Bank,
		FWaitEndBankAction* LoadBankLatentAction
	);

	/**
	 * Load a soundbank asynchronously, using a Blueprint Delegate for completion
	 *
	 * @param in_Bank			The bank to load
	 * @param BankLoadedCallback Blueprint Delegate called upon completion
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT LoadBankAsync(
		class UAkAudioBank *     in_Bank,
		const FOnAkBankCallback& BankLoadedCallback,
		AkMemPoolId         in_memPoolId,
		AkBankID &          out_bankID
	);

	/**
	 * Unload a soundbank
	 *
	 * @param in_Bank			The bank to unload
	 * @param out_pMemPoolId	Returned memory pool ID used with LoadBank() (can pass NULL)
	 * @return Result from ak sound engine 
	 */
	AKRESULT UnloadBank(
        class UAkAudioBank *      in_Bank,
        AkMemPoolId *       out_pMemPoolId = NULL
        );

	/**
	 * Unload a soundbank by its name
	 *
	 * @param in_BankName		The name of the bank to unload
	 * @param out_pMemPoolId	Returned memory pool ID used with LoadBank() (can pass NULL)
	 * @return Result from ak sound engine 
	 */
	AKRESULT UnloadBank(
        const FString&      in_BankName,
        AkMemPoolId *       out_pMemPoolId = NULL
        );

	/**
	 * Unload a soundbank asynchronously
	 *
	 * @param in_Bank			The bank to unload
	 * @param in_pfnBankCallback Callback function
	 * @param in_pCookie		Callback cookie (reserved to user, passed to the callback function)
	 * @return Result from ak sound engine 
	 */
	AKRESULT UnloadBank(
        class UAkAudioBank *      in_Bank,
		AkBankCallbackFunc  in_pfnBankCallback,
		void *              in_pCookie
        );

	/**
	 * Unload a soundbank asynchronously, flagging completion in the latent action
	 *
	 * @param in_Bank			The bank to load
	 * @param UnloadBankLatentAction Blueprint Latent action to flag completion
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT UnloadBank(
		class UAkAudioBank *     in_Bank,
		FWaitEndBankAction* UnloadBankLatentAction
	);

	/**
	 * Unload a soundbank asynchronously, using a Blueprint Delegate for completion
	 *
	 * @param in_Bank			The bank to load
	 * @param BankUnloadedCallback Blueprint Delegate called upon completion
	 * @param in_memPoolId		Memory pool ID (media is stored in the sound engine's default pool if AK_DEFAULT_POOL_ID is passed)
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT UnloadBankAsync(
		class UAkAudioBank *     in_Bank,
		const FOnAkBankCallback& BankUnloadedCallback
	);

	/**
	 * Load the audiokinetic 'init' bank
	 *
	 * @return Result from ak sound engine 
	 */
	AKRESULT LoadInitBank(void);

	/**
	* Load all file packages found in the SoundBanks base path
	*
	* @return operation success
	*/
	bool LoadAllFilePackages(void);

	/**
	 * Unload the audiokinetic 'init' bank
	 *
	 * @return Result from ak sound engine 
	 */
	AKRESULT UnloadInitBank(void);

	/**
	* Unload all file packages found in the SoundBanks base path
	*
	* @return operation success
	*/
	bool UnloadAllFilePackages(void);

	/**
	 * Load all banks currently being referenced
	 */
	void LoadAllReferencedBanks(void);
	
	/**
	 * Reload all banks currently being referenced
	 */
	void ReloadAllReferencedBanks(void);

	/**
	 * FString-friendly GetIDFromString
	 */
	AkUInt32 GetIDFromString(const FString& in_string);

	/**
	 * Post an event to ak soundengine
	 *
	 * @param in_pEvent			Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param in_uFlags			Bitmask: see \ref AkCallbackType
	 * @param in_pfnCallback	Callback function
	 * @param in_pCookie		Callback cookie that will be sent to the callback function along with additional information.
	 * @param in_bStopWhenOwnerDestroyed If true, then the sound should be stopped if the owning actor is destroyed
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEvent(
		class UAkAudioEvent * in_pEvent,
		AActor * in_pActor,
        AkUInt32 in_uFlags = 0,
		AkCallbackFunc in_pfnCallback = NULL,
		void * in_pCookie = NULL,
		bool in_bStopWhenOwnerDestroyed = false,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
        );

	/**
	 * Post an event to ak soundengine by name
	 *
	 * @param in_EventName		Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param in_uFlags			Bitmask: see \ref AkCallbackType
	 * @param in_pfnCallback	Callback function
	 * @param in_pCookie		Callback cookie that will be sent to the callback function along with additional information.
	 * @param in_bStopWhenOwnerDestroyed If true, then the sound should be stopped if the owning actor is destroyed
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEvent(
		const FString& in_EventName, 
		AActor * in_pActor,
		AkUInt32 in_uFlags = 0,
		AkCallbackFunc in_pfnCallback = NULL,
		void * in_pCookie = NULL,
		bool in_bStopWhenOwnerDestroyed = false,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
		);

	/**
	 * Post an event to ak soundengine by name
	 *
	 * @param in_EventName		Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param PostEventCallback	Callback delegate
	 * @param in_uFlags			Bitmask: see \ref EAkCallbackType
	 * @param in_bStopWhenOwnerDestroyed If true, then the sound should be stopped if the owning actor is destroyed
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEvent(
		const FString& in_EventName,
		AActor * in_pActor,
		const FOnAkPostEventCallback& PostEventCallback,
		AkUInt32 in_uFlags = 0,
		bool in_bStopWhenOwnerDestroyed = false,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
	);

	/**
	 * Post an event to ak soundengine by name
	 *
	 * @param in_EventName		Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param in_bStopWhenOwnerDestroyed If true, then the sound should be stopped if the owning actor is destroyed
	 * @param LatentAction		Pointer to a Blueprint latent action.Used in the EndOfEvent callback.
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEventLatentAction(
		const FString& EventName,
		AActor * Actor,
		bool bStopWhenOwnerDestroyed,
		FWaitEndOfEventAction* LatentAction,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
		);


	/**
	 * Post an event to ak soundengine by name
	 *
	 * @param in_EventName		Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param in_uFlags			Bitmask: see \ref AkCallbackType
	 * @param in_pfnCallback	Callback function
	 * @param in_pCookie		Callback cookie that will be sent to the callback function along with additional information.
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEvent(
		const FString& in_EventName,
		UAkComponent* in_pComponent,
		AkUInt32 in_uFlags = 0,
		AkCallbackFunc in_pfnCallback = NULL,
		void * in_pCookie = NULL,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
	);

	/**
	 * Post an event to ak soundengine by name
	 *
	 * @param in_EventName		Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param PostEventCallback	Callback delegate
	 * @param in_uFlags			Bitmask: see \ref EAkCallbackType
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEvent(
		const FString& in_EventName,
		UAkComponent* in_pComponent,
		const FOnAkPostEventCallback& PostEventCallback,
		AkUInt32 in_uFlags = 0,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
	);

	/**
	 * Post an event to ak soundengine by name
	 *
	 * @param in_EventName		Name of the event to post
	 * @param in_pComponent		AkComponent on which to play the event
	 * @param LatentAction		Pointer to a Blueprint latent action. Used in the EndOfEvent callback.
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEventLatentAction(
		const FString& in_EventName,
		UAkComponent* in_pComponent,
		FWaitEndOfEventAction* LatentAction,
		const TArray<AkExternalSourceInfo>& in_ExternalSources = TArray<AkExternalSourceInfo>()
	);

	/**
	 * Post an event at location to ak soundengine
	 *
	 * @param in_pEvent			Name of the event to post
	 * @param in_Location		Location at which to play the event
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEventAtLocation(
		class UAkAudioEvent * in_pEvent,
		FVector in_Location,
		FRotator in_Orientation,
		class UWorld* in_World
		);

	/**
	 * Post an event by name at location to ak soundengine
	 *
	 * @param in_pEvent			Name of the event to post
	 * @param in_Location		Location at which to play the event
	 * @return ID assigned by ak soundengine
	 */
	AkPlayingID PostEventAtLocation(
		const FString& in_EventName,
		FVector in_Location,
		FRotator in_Orientation,
		class UWorld* in_World
		);

	/** Spawn an AkComponent at a location. Allows, for example, to set a switch on a fire and forget sound.
	 * @param AkEvent - Wwise Event to post.
	 * @param EarlyReflectionsBus - Use the provided auxiliary bus to process early reflections.  If NULL, EarlyReflectionsBus will be used.
	 * @param Location - Location from which to post the Wwise Event.
	 * @param Orientation - Orientation of the event.
	 * @param AutoPost - Automatically post the event once the AkComponent is created.
	 * @param EarlyReflectionsBusName - Use the provided auxiliary bus to process early reflections.  If empty, no early reflections will be processed.
	 * @param AutoDestroy - Automatically destroy the AkComponent once the event is finished.
	 */
	class UAkComponent* SpawnAkComponentAtLocation( class UAkAudioEvent* in_pAkEvent, class UAkAuxBus* EarlyReflectionsBus, FVector Location, FRotator Orientation, bool AutoPost, const FString& EventName, const FString& EarlyReflectionsBusName, bool AutoDestroy, class UWorld* in_World );

	/**
	 * Executes an action on all nodes that are referenced in the specified event in an action of type play.
	 * @param in_EventName Name of the event to post
	 * @param in_ActionType Action to execute on all the elements that were played using the specified event.
	 * @param in_pActor Associated actor
	 * @param in_uTransitionDuration Fade duration
	 * @param in_eFadeCurve Curve type to be used for the transition
	 * @param in_PlayingID Associated PlayingID
	 */
	AKRESULT ExecuteActionOnEvent(
		const FString& in_EventName,
		AkActionOnEventType in_ActionType,
		AActor* in_pActor,
		AkTimeMs in_uTransitionDuration = 0,
		EAkCurveInterpolation in_eFadeCurve = EAkCurveInterpolation::Linear,
		AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID
	);

	/**
	 * Executes an Action on the content associated to the specified playing ID. 
	 * @param in_ActionType Action to execute on the specified playing ID.
	 * @param in_PlayingID Playing ID on which to execute the action.
	 * @param in_uTransitionDuration Fade duration
	 * @param in_eFadeCurve Curve type to be used for the transition
	 */
	void ExecuteActionOnPlayingID(
		AkActionOnEventType in_ActionType,
		AkPlayingID in_PlayingID,
		AkTimeMs in_uTransitionDuration = 0,
		EAkCurveInterpolation in_eFadeCuve = EAkCurveInterpolation::Linear
	);

    /** Seek on an event in the ak soundengine.
    * @param in_EventName            Name of the event on which to seek.
    * @param in_pComponent           The associated Actor.
    * @param in_iPosition            Desired percent where playback should restart.
    * @param in_bSeekToNearestMarker If true, the final seeking position will be made equal to the nearest marker.
    *
    * @return Success or failure.
    */
    AKRESULT SeekOnEvent(
        const FString& in_EventName,
        AActor* in_pActor,
        AkReal32 in_fPercent,
        bool in_bSeekToNearestMarker = false,
        AkPlayingID InPlayingID = AK_INVALID_PLAYING_ID
    );

    /** Seek on an event in the ak soundengine.
    * @param in_EventName            Name of the event on which to seek.
    * @param in_pComponent           The associated AkComponent.
    * @param in_fPercent             Desired percent where playback should restart.
    * @param in_bSeekToNearestMarker If true, the final seeking position will be made equal to the nearest marker.
    *
    * @return Success or failure.
    */
    AKRESULT SeekOnEvent(
        const FString& in_EventName,
        UAkComponent* in_pComponent,
        AkReal32 in_fPercent,
        bool in_bSeekToNearestMarker = false,
        AkPlayingID InPlayingID = AK_INVALID_PLAYING_ID
    );

	/**
	 * Post a trigger to ak soundengine
	 *
	 * @param in_pszTrigger		Name of the trigger
	 * @param in_pAkComponent	AkComponent on which to post the trigger
	 * @return Result from ak sound engine
	 */
	AKRESULT PostTrigger( 
		const TCHAR * in_pszTrigger,
		AActor * in_pActor
		);

	/**
	 * Set a RTPC in ak soundengine
	 *
	 * @param in_pszRtpcName	Name of the RTPC
	 * @param in_value			Value to set
	 * @param in_interpolationTimeMs - Duration during which the RTPC is interpolated towards in_value (in ms)
	 * @param in_pActor			AActor on which to set the RTPC
	 * @return Result from ak sound engine
	 */
	AKRESULT SetRTPCValue( 
		const TCHAR * in_pszRtpcName,
		AkRtpcValue in_value,
		int32 in_interpolationTimeMs,
		AActor * in_pActor
		);

	/**
	*  Get the value of a real-time parameter control (by ID)
	*  An RTPC can have a any combination of a global value, a unique value for each game object, or a unique value for each playing ID.
	*  The value requested is determined by RTPCValue_type, in_gameObjectID and in_playingID.
	*  If a value at the requested scope (determined by RTPCValue_type) is not found, the value that is available at the the next broadest scope will be returned, and io_rValueType will be changed to indicate this.
	*  @note
	* 		When looking up RTPC values via playing ID (ie. io_rValueType is RTPC_PlayingID), in_gameObjectID can be set to a specific game object (if it is available to the caller) to use as a fall back value.
	* 		If the game object is unknown or unavailable, AK_INVALID_GAME_OBJECT can be passed in in_gameObjectID, and the game object will be looked up via in_playingID.
	* 		However in this case, it is not possible to retrieve a game object value as a fall back value if the playing id does not exist.  It is best to pass in the game object if possible.
	*
	*  @return AK_Success if succeeded, AK_IDNotFound if the game object was not registered, or AK_Fail if the RTPC value could not be obtained
	*/
	AKRESULT GetRTPCValue(
		const TCHAR * in_pszRtpcName,
		AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
		AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
		AkRtpcValue& out_rValue, 			///< Value returned
		AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
	);

	/**
	 * Set a state in ak soundengine
	 *
	 * @param in_pszStateGroup	Name of the state group
	 * @param in_pszState		Name of the state
	 * @return Result from ak sound engine
	 */
	AKRESULT SetState( 
		const TCHAR * in_pszStateGroup,
		const TCHAR * in_pszState
	    );
		
	/**
	 * Set a switch in ak soundengine
	 *
	 * @param in_pszSwitchGroup	Name of the switch group
	 * @param in_pszSwitchState	Name of the switch
	 * @param in_pComponent		AkComponent on which to set the switch
	 * @return Result from ak sound engine
	 */
	AKRESULT SetSwitch( 
		const TCHAR * in_pszSwitchGroup,
		const TCHAR * in_pszSwitchState,
		AActor * in_pActor
		);

    /** Sets multiple positions to a single game object.
    *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
    *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
    *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
    *  @param in_pGameObjectAkComponent UAkComponent of the game object.
    *  @param in_aPositions Array of positions to apply.
    *  @param in_eMultiPositionType Position type
    *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
    */
    AKRESULT SetMultiplePositions(
        UAkComponent* in_pGameObjectAkComponent,
        TArray<FTransform> in_aPositions,
        AkMultiPositionType in_eMultiPositionType = AkMultiPositionType::MultiDirections
    );

    /** Sets multiple positions to a single game object, with flexible assignment of input channels.
    *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
    *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
    *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
    *  @param in_pGameObjectAkComponent Game Object AkComponent.
    *  @param in_aChannelConfigurations Array of channel configurations for each position.
    *  @param in_pPositions Array of positions to apply.
    *  @param in_eMultiPositionType Position type
    *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
    */
    AKRESULT SetMultiplePositions(
        UAkComponent* in_pGameObjectAkComponent,
        const TArray<AkChannelConfiguration>& in_aChannelConfigurations,
        const TArray<FTransform>& in_aPositions,
        AkMultiPositionType in_eMultiPositionType = AkMultiPositionType::MultiDirections
    );

	/** Sets multiple positions to a single game object, with flexible assignment of input channels.
	*  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
	*  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
	*  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
	*  @param in_pGameObjectAkComponent Game Object AkComponent.
	*  @param in_channelMasks Array of channel mask for each position.
	*  @param in_pPositions Array of positions to apply.
	*  @param in_eMultiPositionType Position type
	*  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
	*/
	AKRESULT SetMultiplePositions(
		UAkComponent* in_pGameObjectAkComponent,
		const TArray<FAkChannelMask>& in_channelMasks,
		const TArray<FTransform>& in_aPositions,
		AkMultiPositionType in_eMultiPositionType = AkMultiPositionType::MultiDirections
	);

    /** Sets multiple positions to a single game object.
     *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
     *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
     *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
     *  @param in_GameObjectID Game Object identifier.
     *  @param in_pPositions Array of positions to apply.
     *  @param in_NumPositions Number of positions specified in the provided array.
     *  @param in_eMultiPositionType Position type
     *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
     *  
     */
    AKRESULT SetMultiplePositions(
        AkGameObjectID in_GameObjectID,						
        const AkSoundPosition * in_pPositions,				
        AkUInt16 in_NumPositions,							
        AK::SoundEngine::MultiPositionType in_eMultiPositionType = AK::SoundEngine::MultiPositionType_MultiDirections
        );

    /** Sets multiple positions to a single game object, with flexible assignment of input channels.
     *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
     *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
     *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
     *  @param in_GameObjectID Game Object identifier.
     *  @param in_pPositions Array of positions to apply.
     *  @param in_NumPositions Number of positions specified in the provided array.
     *  @param in_eMultiPositionType Position type
     *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
     */
    AKRESULT SetMultiplePositions(
        AkGameObjectID in_GameObjectID,
        const AkChannelEmitter * in_pPositions,
        AkUInt16 in_NumPositions,
        AK::SoundEngine::MultiPositionType in_eMultiPositionType = AK::SoundEngine::MultiPositionType_MultiDirections
        );

	/**
	 * Set auxiliary sends
	 *
	 * @param in_GameObjId		Wwise Game Object ID
	 * @param in_AuxSendValues	Array of AkAuxSendValue, containins all Aux Sends to set on the game objectt
	 * @return Result from ak sound engine
	 */
	AKRESULT SetAuxSends(
		const UAkComponent* in_akComponent,
		TArray<AkAuxSendValue>& in_AuxSendValues
		);

	/**
	* Set spatial audio room
	*
	* @param in_GameObjId		Wwise Game Object ID
	* @param in_RoomID	ID of the room that the game object is inside.
	* @return Result from ak sound engine
	*/
	AKRESULT SetInSpatialAudioRoom(
		const AkGameObjectID in_GameObjId,
		AkRoomID in_RoomID
	);

	/**
	 * Force channel configuration for the specified bus.
	 * This function has unspecified behavior when changing the configuration of a bus that 
	 * is currently playing.
	 * You cannot change the configuration of the master bus.
	 *
	 * @param in_BusName	Bus Name
	 * @param in_Config		Desired channel configuration. An invalid configuration (from default constructor) means "as parent".
	 * @return Always returns AK_Success
	 */
	AKRESULT SetBusConfig(
		const FString&	in_BusName,
		AkChannelConfig	in_Config
		);

	/**
	 *  Set the panning rule of the specified output.
	 *  This may be changed anytime once the sound engine is initialized.
	 *  \warning This function posts a message through the sound engine's internal message queue, whereas GetPanningRule() queries the current panning rule directly.
	 */
	AKRESULT SetPanningRule(
		AkPanningRule		in_ePanningRule			///< Panning rule.
		);

	/**
	 * Gets the compounded output ID from shareset and device id.
	 * Outputs are defined by their type (Audio Device shareset) and their specific system ID.
	 * A system ID could be reused for other device types on some OS or platforms, hence the compounded ID.
	 *
	 * @param in_szShareset Audio Device ShareSet Name, as defined in the Wwise Project.  If Null, will select the Default Output shareset (always available)
	 * @param in_idDevice Device specific identifier, when multiple devices of the same type are possible. If only one device is possible, leave to 0.
	 * @return The id of the output
	 */
	AkOutputDeviceID GetOutputID(
		const FString& in_szShareSet,
		AkUInt32 in_idDevice = 0
	);

	/**
	 * Gets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
	 * Note that the current version of Wwise only supports positioning on the plane.
	 * The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
	 * Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
	 * Angles must be set in ascending order.
	 * Typical usage:
	 * - AkReal32 heightAngle;
	 * - TArray<AkReal32> speakerAngles;
	 * - GetSpeakerAngles(speakerAngles, heightAngle, AkOutput_Main );
	 * \aknote
	 *  On most platforms, the angle set on the plane consists of 3 angles, to account for 7.1.
	 * - When panning to stereo (speaker mode, see <tt>AK::SoundEngine::SetPanningRule()</tt>), only angle[0] is used, and 3D sounds in the back of the listener are mirrored to the front.
	 * - When panning to 5.1, the front speakers use angle[0], and the surround speakers use (angle[2] - angle[1]) / 2.
	 * \endaknote
	 * \warning Call this function only after the sound engine has been properly initialized.
	 *
	 * @param io_pfSpeakerAngles Returned array of loudspeaker pair angles, in degrees relative to azimuth [0,180]. Pass NULL to get the required size of the array.
	 * @param out_fHeightAngle Elevation of the height layer, in degrees relative to the plane [-90,90].
	 * @param in_idOutput Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output
	 * @return AK_Success if device exists
	 *
	 */
	AKRESULT GetSpeakerAngles(
		TArray<AkReal32>& io_pfSpeakerAngles,
		AkReal32& out_fHeightAngle,
		AkOutputDeviceID in_idOutput = 0
		);

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
	 * @param in_pfSpeakerAngles Array of loudspeaker pair angles, in degrees relative to azimuth [0,180]
	 * @param in_fHeightAngle Elevation of the height layer, in degrees relative to the plane [-90,90]
	 * @param in_idOutput Output ID to set the bus on. As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output
	 * @return AK_Success if successful (device exists and angles are valid), AK_NotCompatible if the channel configuration of the device is not standard (AK_ChannelConfigType_Standard), AK_Fail otherwise.
	 *
	 */
	AKRESULT SetSpeakerAngles(
		const TArray<AkReal32>& in_pfSpeakerAngles,
		AkReal32 in_fHeightAngle,
		AkOutputDeviceID in_idOutput = 0
		);

	/**
	 * Set the output bus volume (direct) to be used for the specified game object.
	 * The control value is a number ranging from 0.0f to 1.0f.
	 *
	 * @param in_GameObjId		Wwise Game Object ID
	 * @param in_fControlValue	Control value to set
	 * @return	Always returns Ak_Success
	 */
	AKRESULT SetGameObjectOutputBusVolume(
		const UAkComponent* in_pEmitter,
		const UAkComponent* in_pListener,
		float in_fControlValue
		);

	/**
	 * Registers a callback that can run within the global callback at a specific AkGlobalCallbackLocation.
	 *
	 * @param Callback		The callback that will be called.
	 * @param Location		The location in the sound engine processing loop
	 * @return	Returns the handle of the delegate that must be used to unregister the callback.
	 */
	FDelegateHandle RegisterGlobalCallback(FAkAudioDeviceDelegates::FOnAkGlobalCallback::FDelegate Callback, AkGlobalCallbackLocation Location);

	/**
	 * Unregisters a callback that can run within the global callback at a specific AkGlobalCallbackLocation.
	 *
	 * @param Handle		The handle of the registered callback
	 * @param Location		The location in the sound engine processing loop
	 */
	void UnregisterGlobalCallback(FDelegateHandle Handle, AkGlobalCallbackLocation Location);

	/**
	 * Obtain a pointer to the singleton instance of FAkAudioDevice
	 *
	 * @return Pointer to the singleton instance of FAkAudioDevice
	 */
	static FAkAudioDevice * Get();

	/**
	 * Stop all audio associated with a game object
	 *
	 * @param in_pComponent		AkComponent which should be stopped
	 */
	void StopGameObject(UAkComponent * in_pComponent);

	/**
	 * Stop all audio associated with a playing ID
	 *
	 * @param in_playingID		AkPlayingID which should be stopped
	 */
	void StopPlayingID( AkPlayingID in_playingID,
                        AkTimeMs in_uTransitionDuration = 0,
                        AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear);

	/**
	 * Register an ak audio component with ak sound engine
	 *
	 * @param in_pComponent		Pointer to the component to register
	 */
	void RegisterComponent(UAkComponent * in_pComponent);

	/**
	 * Unregister an ak audio component with ak sound engine
	 *
	 * @param in_pComponent		Pointer to the component to unregister
	 */
	void UnregisterComponent(UAkComponent * in_pComponent);
	
	/**
	* Register an ak audio component with ak spatial audio
	*
	* @param in_pComponent		Pointer to the component to register
	*/
	void RegisterSpatialAudioEmitter(UAkComponent * in_pComponent);

	/**
	* Unregister an ak audio component with ak spatial audio
	*
	* @param in_pComponent		Pointer to the component to unregister
	*/
	void UnregisterSpatialAudioEmitter(UAkComponent * in_pComponent);

	/**
	* Send a set of triangles to the Spatial Audio Engine
	*/
	AKRESULT SetGeometry(AkGeometrySetID AcousticZoneID, const AkGeometryParams& Params);

	/**
	* Remove a set of triangles from the Spatial Audio Engine
	*/
	AKRESULT RemoveGeometrySet(AkGeometrySetID AcousticZoneID);

	/**
	 * Get an ak audio component, or create it if none exists that fit the attachment criteria.
	 */
	static class UAkComponent* GetAkComponent(
		class USceneComponent* AttachToComponent, FName AttachPointName, const FVector * Location, EAttachLocation::Type LocationType);

	static class UAkComponent* GetAkComponent(
		class USceneComponent* AttachToComponent, FName AttachPointName, const FVector * Location, EAttachLocation::Type LocationType, bool& ComponentCreated);

	/**
	 * Cancel the callback cookie for a dispatched event 
	 *
	 * @param in_cookie			The cookie to cancel
	 */
	void CancelEventCallbackCookie(void* in_cookie);

	void CancelEventCallbackDelegate(const FOnAkPostEventCallback& in_Delegate);

	 /** 
	  * Set the scaling factor of a game object.
	  * Modify the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect.
	  */
	AKRESULT SetAttenuationScalingFactor(AActor* Actor, float ScalingFactor);

	 /** 
	  * Set the scaling factor of a AkComponent.
	  * Modify the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect.
	  */
	AKRESULT SetAttenuationScalingFactor(UAkComponent* AkComponent, float ScalingFactor);

	/**
	 * Starts a Wwise output capture. The output file will be located in the same folder as the SoundBanks.
	 * @param Filename - The name to give to the output file.
	 */
	void StartOutputCapture(const FString& Filename);

	/**
	 * Add text marker in output capture file.
	 * @param MarkerText - The name text to put in the marker.
	 */
	void AddOutputCaptureMarker(const FString& MarkerText);

	/**
	 * Stops a Wwise output capture. The output file will be located in the same folder as the SoundBanks.
	 */
	void StopOutputCapture();

	/**
	 * Starts a Wwise profiler capture. The output file will be located in the same folder as the SoundBanks.
	 * @param Filename - The name to give to the output file.
	 */
	void StartProfilerCapture(const FString& Filename);

	/**
	 * Stops a Wwise profiler capture. The output file will be located in the same folder as the SoundBanks.
	 */
	void StopProfilerCapture();

	/**
	 * Allows to register a Wwise plugin from a DLL name and path
	 */
	AKRESULT RegisterPluginDLL(const FString& in_DllName, const FString& in_DllPath);

	/**
	* Gets the path where the SoundBanks are located on disk
	*/
	FString GetBasePath();

	/**
	 * Suspends the SoundEngine
	 * @param in_bRenderAnyway	If set to true, audio processing will still occur, but not outputted. When set to false, no audio will be processed at all, even upon reception of RenderAudio().
	 */
	void Suspend(bool in_bRenderAnyway = false);

	/**
	 * Return from a suspended state
	 */
	void WakeupFromSuspend();

    /***
    * Event called when soundbanks are generated
    */
    SoundbanksLoaded OnSoundbanksLoaded;

#ifdef AK_SOUNDFRAME
	/**
	 * Called when sound frame connects 
	 *
	 * @param in_bConnect		True if Wwise is connected, False if it is not
	 */	
	virtual void OnConnect( 
		bool in_bConnect		///< True if Wwise is connected, False if it is not
		);
		
	/**
	 * Event notification. This method is called when an event is added, removed, changed, or pushed.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_eventID		Unique ID of the event
	 */	
	virtual void OnEventNotif( 
		Notif in_eNotif,
		AkUniqueID in_eventID
		);
			
	/**
	 * SoundBank notification. This method is called when a soundbank is added, removed, or changed.
	 */
	virtual void OnSoundBankNotif(
		Notif in_eNotif,			///< Notification type
		AkUniqueID in_AuxBusID		///< Unique ID of the auxiliary bus
		) {};

	/**
	 * Dialogue Event notification. This method is called when a dialogue event is added, removed or changed.
	 *
	 * This notification will be sent if an argument is added, removed or moved within a dialogue event.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_eventID		Unique ID of the event
	 */
	virtual void OnDialogueEventNotif( 
		Notif in_eNotif,
		AkUniqueID in_dialogueEventID
		) {}
			
	/**
	 * Sound object notification. This method is called when a sound object is added, removed, or changed.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_soundObjectID	Unique ID of the sound object
	 */
	virtual void OnSoundObjectNotif( 
		Notif in_eNotif,
		AkUniqueID in_soundObjectID
		);
		
	/**
	 * State notification.
	 *
	 * This method is called when a state group or a state is added, removed or changed.
	 * It is also called (with in_eNotif equal to Notif_Changed) when the current state of a state group changes.
	 *
	 * This notification will be sent for all state changes (through Wwise, the Sound Frame, or the sound engine).
	 *
	 * @param in_eNotif			Notification type
	 * @param in_stateGroupID	Unique ID of the state group
	 */
	virtual void OnStatesNotif( 
		Notif in_eNotif,
		AkUniqueID in_stateGroupID
		) {}
			
	/**
	 * Switch notification.
	 *
	 * This method is called when a switch group or a switch is added, removed or changed.
	 * It is also called (with in_eNotif equal to Notif_Changed) when the current switch in a switch group changes on any game object.
	 *
	 * This notification will be sent for all switch changes (through Wwise, the Sound Frame, or the sound engine).
	 *
	 * @param in_eNotif			Notification type
	 * @param in_switchGroupID	Unique ID of the switch group
	 */
	virtual void OnSwitchesNotif( 
		Notif in_eNotif,			///< Notification type
		AkUniqueID in_switchGroupID	///< Unique ID of the switch group
		) {}
			
	/**
	 * Game parameter notification.
	 *
	 * This method is called when a game parameter is added, removed, or changed.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_gameParameterID Unique ID of the game parameter
	 */
	virtual void OnGameParametersNotif( 
		Notif in_eNotif,
		AkUniqueID in_gameParameterID
		) {}
			
	/**
	 * Trigger notification.
	 *
	 * This method is called when a trigger is added, removed, or changed.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_triggerID		Unique ID of the trigger
	 */
	virtual void OnTriggersNotif( 
		Notif in_eNotif,
		AkUniqueID in_triggerID
		) {}
				
	/**
	 * Argument notification.
	 *
	 * This method is called when an argument or argument value is added, removed, or changed.
	 * Although this notification is called when an argument is created, you will probably be more interested to 
	 * know when this argument gets referenced by a dialogue event. See OnDialogueEventNotif().
	 *
	 * @param in_eNotif			Notification type
	 * @param in_argumentID		Unique ID of the argument
	 */
	virtual void OnArgumentsNotif( 
		Notif in_eNotif,
		AkUniqueID in_argumentID
		) {}
			
	/**
	 * Aux bus notification.
	 *
	 * This method is called when an aux bus is added, removed, or changed.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_AuxBusID		Unique ID of the aux bus
	 */
	virtual void OnAuxBusNotif( 
		Notif in_eNotif,     
		AkUniqueID in_AuxBusID
		) {}
			
	/**
	 * Game object notification.
	 *
	 * This method is called when a game object is registered or unregistered.
	 * The notification type will be Notif_Added when a game object is registered, and Notif_Removed 
	 * when its unregistered.
	 * - This notification will be sent for game object registration and unregistration made through the Sound Frame 
	 * or the sound engine.
	 * - The notification type will be Notif_Reset when all game objects are removed from the Sound Engine.
	 *
	 * @param in_eNotif			Notification type
	 * @param in_gameObjectID	Unique ID of the game object
	 */
	virtual void OnGameObjectsNotif( 
		Notif in_eNotif,
		AkGameObjectID in_gameObjectID
		) {}

	/**
	 * Obtain a pointer to sound frame
	 *
	 * @return A pointer to sound frame
	 */
	AK::SoundFrame::ISoundFrame * GetSoundFrame(void) { return m_pSoundFrame; }
#endif

	static inline void FVectorToAKVector( const FVector & in_vect, AkVector & out_vect )
	{
		out_vect.X = in_vect.X;
		out_vect.Y = in_vect.Y;
		out_vect.Z = in_vect.Z;
	}

	static inline AkVector FVectorToAKVector(const FVector & in_vect)
	{
		return AkVector{ in_vect.X, in_vect.Y, in_vect.Z };
	}

	static inline void FVectorsToAKTransform(const FVector& in_Position, const FVector& in_Front, const FVector& in_Up, AkTransform& out_AkTransform)
	{
		// Convert from the UE axis system to the Wwise axis system
		out_AkTransform.Set(FVectorToAKVector(in_Position), FVectorToAKVector(in_Front), FVectorToAKVector(in_Up));
	}

	static inline void AKVectorToFVector(const AkVector & in_vect, FVector & out_vect)
	{
		out_vect.X = in_vect.X;
		out_vect.Y = in_vect.Y;
		out_vect.Z = in_vect.Z;
	}

	static inline FVector AKVectorToFVector(const AkVector& in_vect)
	{
		return FVector(in_vect.X, in_vect.Y, in_vect.Z);
	}

	FAkBankManager * GetAkBankManager()
	{
		return AkBankManager;
	}

	uint8 GetMaxAuxBus()
	{
		return MaxAuxBus;
	}

	AkCallbackInfoPool* GetAkCallbackInfoPool()
	{
		return CallbackInfoPool;
	}

#if WITH_EDITOR
	void SetMaxAuxBus(uint8 ValToSet) 
	{
		MaxAuxBus = ValToSet;
	}
#endif

	static const int32 FIND_COMPONENTS_DEPTH_INFINITE = -1;

	/** Find UAkLateReverbComponents at a given location. */
	TArray<class UAkLateReverbComponent*> FindLateReverbComponentsAtLocation(const FVector& Loc, const UWorld* in_World, int32 depth = FIND_COMPONENTS_DEPTH_INFINITE);

	/** Add a UAkLateReverbComponent to the linked list. */
	void AddLateReverbComponentToPrioritizedList(class UAkLateReverbComponent* in_ComponentToAdd);

	/** Remove a UAkLateReverbComponent from the linked list. */
	void RemoveLateReverbComponentFromPrioritizedList(class UAkLateReverbComponent* in_ComponentToRemove);

	/** Get whether the given world has room registered in it. */
	bool WorldHasActiveRooms(UWorld* in_World);

	/** Find UAkRoomComponents at a given location. */
	TArray<class UAkRoomComponent*> FindRoomComponentsAtLocation(const FVector& Loc, const UWorld* in_World, int32 depth = FIND_COMPONENTS_DEPTH_INFINITE);

	/** Return true if any UAkRoomComponents have been added to the prioritized list of rooms for the in_World**/
	bool UsingSpatialAudioRooms(const UWorld* in_World);

	/** Get the aux send values corresponding to a point in the world.**/
	void GetAuxSendValuesAtLocation(FVector Loc, TArray<AkAuxSendValue>& AkAuxSendValues, const UWorld* in_World);

	/** Update all rooms. */
	void UpdateAllSpatialAudioRooms(UWorld* InWorld);

	/** Update all portals. */
	void UpdateAllSpatialAudioPortals(UWorld* InWorld);

	/** Register a Portal in AK Spatial Audio.  Can be called again to update the portal parameters.	*/
	void SetSpatialAudioPortal(const AAkAcousticPortal* in_Portal);
	
	/** Remove a Portal from AK Spatial Audio	*/
	void RemoveSpatialAudioPortal(const AAkAcousticPortal* in_Portal);
	
	void OnActorSpawned(AActor* SpawnedActor);

	UAkComponentSet& GetDefaultListeners() { return m_defaultListeners; }
	UAkComponentSet& GetDefaultEmitters() { return m_defaultEmitters; }

	void SetListeners(UAkComponent* in_pEmitter, const TArray<UAkComponent*>& in_listenerSet);
	void AddDefaultListener(UAkComponent* in_pListener);
	void RemoveDefaultListener(UAkComponent* in_pListener);
	void UpdateDefaultActiveListeners();
#if WITH_EDITORONLY_DATA
	FTransform GetEditorListenerPosition(int32 ViewIndex) const;
#endif

	/** Specifies which listener is used for Wwise Spatial Audio**/
	bool SetSpatialAudioListener(UAkComponent* in_pListener);
	
	/** Get the listener that has been choosen to be used for Wwise Spatial Audio**/
	UAkComponent* GetSpatialAudioListener() const;

	AKRESULT SetPosition(UAkComponent* in_akComponent, const AkTransform& in_SoundPosition);

	/** Add a UAkRoomComponent to the linked list. */
	void AddRoomComponentToPrioritizedList(class UAkRoomComponent* in_ComponentToAdd);

	/** Remove a UAkRoomComponent from the linked list. */
	void RemoveRoomComponentFromPrioritizedList(class UAkRoomComponent* in_ComponentToRemove);

	AKRESULT AddRoom(UAkRoomComponent* in_pRoom, const AkRoomParams& in_RoomParams);
	AKRESULT UpdateRoom(UAkRoomComponent* in_pRoom, const AkRoomParams& in_RoomParams);
	AKRESULT RemoveRoom(UAkRoomComponent* in_pRoom);

	AKRESULT SetImageSource(class AAkSpotReflector* in_pSpotReflector, const AkImageSourceSettings& in_ImageSourceInfo, AkUniqueID in_AuxBusID, AkRoomID in_RoomID);
	AKRESULT RemoveImageSource(class AAkSpotReflector* in_pSpotReflector, AkUniqueID in_AuxBusID);

    static void GetChannelConfig(AkChannelConfiguration ChannelConfiguration, AkChannelConfig& config);
	static void GetChannelConfig(FAkChannelMask SpeakerConfiguration, AkChannelConfig& config);

	TMap<UWorld*, class UAkLateReverbComponent*>& GetHighestPriorityLateReverbComponentMap() { return HighestPriorityLateReverbComponentMap; }
	TMap<UWorld*, class UAkRoomComponent*>& GetHighestPriorityRoomComponentMap() { return HighestPriorityRoomComponentMap; }

	/** Find Components that are prioritized (either UAkLateReverbComponent or UAkRoomComponent) at a given location.**/
	template<class COMPONENT_TYPE>
	TArray<COMPONENT_TYPE*> FindPrioritizedComponentsAtLocation(const FVector& Loc, const UWorld* in_World, TMap<UWorld*, COMPONENT_TYPE*>& HighestPriorityComponentMap, int32 depth = FIND_COMPONENTS_DEPTH_INFINITE);

private:
	bool EnsureInitialized();

	void SetBankDirectory();
	
	void* AllocatePermanentMemory( int32 Size, /*OUT*/ bool& AllocatedInPool );
	
	AKRESULT GetGameObjectID(AActor * in_pActor, AkGameObjectID& io_GameObject );

	template<typename FCreateCallbackPackage>
	AkPlayingID PostEvent(
		const FString& in_EventName,
		const AkGameObjectID in_GameObjectID,
		const TArray<AkExternalSourceInfo>& in_pExternalSources,
		FCreateCallbackPackage CreateCallbackPackage
	);

	template<typename FCreateCallbackPackage>
	AkPlayingID PostEvent(
		const FString& in_EventName,
		UAkComponent* in_pComponent,
		const TArray<AkExternalSourceInfo>& in_pExternalSources,
		FCreateCallbackPackage CreateCallbackPackage
	);

	template<typename ChannelConfig>
	AKRESULT SetMultiplePositions(
		UAkComponent* in_pGameObjectAkComponent,
		const TArray<ChannelConfig>& in_aChannelConfigurations,
		const TArray<FTransform>& in_aPositions,
		AkMultiPositionType in_eMultiPositionType
	);

	// Overload allowing to modify StopWhenOwnerDestroyed after getting the AkComponent
	AKRESULT GetGameObjectID(AActor * in_pActor, AkGameObjectID& io_GameObject, bool in_bStopWhenOwnerDestroyed );

#if WITH_EDITORONLY_DATA
	UAkComponent* CreateListener(UWorld* World, FEditorViewportClient* ViewportClient = nullptr);
	TArray<FTransform> ListenerTransforms;
	UAkComponent* EditorListener = nullptr;
#endif

	/** We keep a linked list of UAkLateReverbComponents sorted by priority for faster finding of reverb volumes at a specific location.
	 *	This points to the highest volume in the list.
	 */
	TMap<UWorld*, class UAkLateReverbComponent*> HighestPriorityLateReverbComponentMap;

	/** We keep a linked list of Spatial audio Rooms sorted by priority for faster finding of reverb volumes at a specific location.
	 *	This points to the highest volume in the list.
	 */
	TMap<UWorld*, class UAkRoomComponent*> HighestPriorityRoomComponentMap;

	void CleanupComponentMapsForWorld(UWorld* World);

#if UE_4_19_OR_LATER
	void CleanupComponentMapsForLevel(ULevel* Level);
	template<class COMPONENT_TYPE>
	void RemovePrioritizedComponentsInLevel(TMap<UWorld*, COMPONENT_TYPE*>& HighestPriorityComponentMap, ULevel* Level);
#else
	template<class COMPONENT_TYPE>
	void RemoveInvalidPrioritizedComponents(TMap<UWorld*, COMPONENT_TYPE*>& HighestPriorityComponentMap);
#endif// UE_4_19_OR_LATER

	/** Add a Component that is prioritized (either UAkLateReverbComponent or UAkRoomComponent) in the active linked list. */
	template<class COMPONENT_TYPE>
	void AddPrioritizedComponentInList(COMPONENT_TYPE* in_ComponentToAdd, TMap<UWorld*, COMPONENT_TYPE*>& HighestPriorityComponentMap);

	/** Remove a Component that is prioritized (either UAkLateReverbComponent or UAkRoomComponent) from the linked list. */
	template<class COMPONENT_TYPE>
	void RemovePrioritizedComponentFromList(COMPONENT_TYPE* in_ComponentToRemove, TMap<UWorld*, COMPONENT_TYPE*>& HighestPriorityComponentMap);

	static bool m_bSoundEngineInitialized;
	UAkComponentSet m_defaultListeners;
	UAkComponentSet m_defaultEmitters;

	UAkComponent* m_SpatialAudioListener;

	bool m_isSuspended = false;

	uint8 MaxAuxBus;

	FAkComponentCallbackManager* CallbackManager;
	AkCallbackInfoPool* CallbackInfoPool;
	FAkBankManager* AkBankManager;
	CAkFilePackageLowLevelIO<CAkUnrealIOHookDeferred, CAkDiskPackage, AkFileCustomParamPolicy>* LowLevelIOHook;

	static bool m_EngineExiting;

#ifdef AK_SOUNDFRAME
	class AK::SoundFrame::ISoundFrame * m_pSoundFrame;
#endif
};
