// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkBank.h:
=============================================================================*/
#pragma once

#include "Engine/GameEngine.h"
#include "AkGameplayTypes.h"
#include "AkAudioBank.generated.h"

/*------------------------------------------------------------------------------------
	UAkAudioBank
------------------------------------------------------------------------------------*/
UCLASS(meta=(BlueprintSpawnableComponent))
class AKAUDIO_API UAkAudioBank : public UObject
{
	GENERATED_UCLASS_BODY() 

public:
	/** Auto-load bank when its package is accessed for the first time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Behaviour)
	bool	AutoLoad;
	
#if CPP
	/**
	 * Called after load process is complete.
	 */
	virtual void PostLoad() override;

	/**
	 * Clean up.
	 */
	virtual void BeginDestroy() override;
	
	/**
	 * Loads an AkBank.
	 *
	 * @return Returns true if the load was successful, otherwise false
	 */
	bool Load();

	/**
	* Loads an AkBank, using the latent action to flag completion.
	*/
	bool Load(FWaitEndBankAction* LoadBankLatentAction);

	/**
	 * Loads an AkBank asynchronously.
	 *
	 * @param in_pfnBankCallback		Function to call on completion
	 * @param in_pCookie				Cookie to pass in callback
	 * @return Returns true if the load was successful, otherwise false
	 */
	bool LoadAsync(void* in_pfnBankCallback, void* in_pCookie);

	/**
	* Loads an AkBank asynchronously, from Blueprint
	*
	* @param BankLoadedCallback		Blueprint Delegate to call on completion
	* @return Returns true if the load was successful, otherwise false
	*/
	bool LoadAsync(const FOnAkBankCallback& BankLoadedCallback);
	
	/**
	 * Unloads an AkBank.
	 */
	void Unload();

	/**
	* Unloads an AkBank, using the latent action to flag completion.
	*/
	void Unload(FWaitEndBankAction* LoadBankLatentAction);
		
	/**
	 * Unloads an AkBank asynchronously.
	 *
	 * @param in_pfnBankCallback		Function to call on completion
	 * @param in_pCookie				Cookie to pass in callback
	 */
	void UnloadAsync(void* in_pfnBankCallback, void* in_pCookie);

	/**
	* Unloads an AkBank asynchronously, from Blueprint
	*
	* @param BankUnloadedCallback		Blueprint Delegate to call on completion
	*/
	void UnloadAsync(const FOnAkBankCallback& BankUnloadedCallback);
#endif
};
