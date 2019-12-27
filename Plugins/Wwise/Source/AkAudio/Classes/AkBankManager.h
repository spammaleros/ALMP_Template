// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkAudioDevice.h: Audiokinetic audio interface object.
=============================================================================*/

#pragma once

/*------------------------------------------------------------------------------------
	AkAudioDevice system headers
------------------------------------------------------------------------------------*/


#include "AkInclude.h"
#include "AkGameplayTypes.h"
#include "Engine/EngineBaseTypes.h"

/*------------------------------------------------------------------------------------
	Audiokinetic SoundBank Manager.
------------------------------------------------------------------------------------*/
class IAkBankCallbackInfo
{
public:
	class UAkAudioBank *	Bank;

	IAkBankCallbackInfo(class UAkAudioBank * bank)
		: Bank(bank)
	{}

	virtual ~IAkBankCallbackInfo() {}

	virtual void HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId) = 0;
};

class FAkBankFunctionPtrCallbackInfo : public IAkBankCallbackInfo
{
public:
	FAkBankFunctionPtrCallbackInfo(AkBankCallbackFunc cbFunc, class UAkAudioBank * bank, void* cookie)
		: IAkBankCallbackInfo(bank)
		, CallbackFunc(cbFunc)
		, UserCookie(cookie)
	{}

	virtual void HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId) override;

private:
	AkBankCallbackFunc		CallbackFunc;
	void*					UserCookie;

};

class FAkBankLatentActionCallbackInfo : public IAkBankCallbackInfo
{
public:
	FAkBankLatentActionCallbackInfo(class UAkAudioBank * bank, FWaitEndBankAction* LatentAction)
		: IAkBankCallbackInfo(bank)
		, BankLatentAction(LatentAction)
	{}

	virtual void HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId) override;

private:
	FWaitEndBankAction* BankLatentAction;
};

class FAkBankBlueprintDelegateCallbackInfo : public IAkBankCallbackInfo
{
public:
	FAkBankBlueprintDelegateCallbackInfo(class UAkAudioBank * bank, const FOnAkBankCallback& BlueprintCallback)
		: IAkBankCallbackInfo(bank)
		, BankBlueprintCallback(BlueprintCallback)
	{}

	virtual void HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId) override;

private:
	FOnAkBankCallback BankBlueprintCallback;
};

class AKAUDIO_API FAkBankManager
{
public:
	FAkBankManager();
	~FAkBankManager();

	static FAkBankManager* GetInstance();

	static void BankLoadCallback(
		AkUInt32		in_bankID,
		const void *	in_pInMemoryBankPtr,
		AKRESULT		in_eLoadResult,
		AkMemPoolId		in_memPoolId,
		void *			in_pCookie
	);

	static void BankUnloadCallback(
		AkUInt32		in_bankID,
		const void *	in_pInMemoryBankPtr,
		AKRESULT		in_eUnloadResult,
		AkMemPoolId		in_memPoolId,
		void *			in_pCookie
	);

	void AddLoadedBank(class UAkAudioBank * Bank)
	{
		bool bIsAlreadyInSet = false;
		m_LoadedBanks.Add(Bank, &bIsAlreadyInSet);
		check(bIsAlreadyInSet == false);
	}

	void RemoveLoadedBank(class UAkAudioBank * Bank)
	{
		m_LoadedBanks.Remove(Bank);
	}

	void ClearLoadedBanks()
	{
		m_LoadedBanks.Empty();
	}

	const TSet<class UAkAudioBank *>* GetLoadedBankList()
	{
		return &m_LoadedBanks;
	}

	FCriticalSection m_BankManagerCriticalSection;

private:
	static FAkBankManager* Instance;
	TSet< class UAkAudioBank * > m_LoadedBanks;
};