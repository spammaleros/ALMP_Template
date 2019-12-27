// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

#include "AkBankManager.h"
#include "AkAudioDevice.h"
#include "AkInclude.h"
#include "AkAudioBank.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"

FAkBankManager* FAkBankManager::Instance = nullptr;

FAkBankManager* FAkBankManager::GetInstance()
{
	return Instance;
}

void FAkBankFunctionPtrCallbackInfo::HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId)
{
	if (CallbackFunc != nullptr)
	{
		// Call the user's callback function
		CallbackFunc(BankID, InMemoryBankPtr, ActionResult, MemPoolId, UserCookie);
	}
}

void FAkBankLatentActionCallbackInfo::HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId)
{
	if (BankLatentAction != nullptr)
	{
		BankLatentAction->ActionDone = true;
	}
}

void FAkBankBlueprintDelegateCallbackInfo::HandleAction(AkUInt32 BankID, const void * InMemoryBankPtr, AKRESULT ActionResult, AkMemPoolId MemPoolId)
{
	if (BankBlueprintCallback.IsBound())
	{
		auto CachedBlueprintCallback = BankBlueprintCallback;
		AsyncTask(ENamedThreads::GameThread, [ActionResult, CachedBlueprintCallback]()
		{
			CachedBlueprintCallback.ExecuteIfBound((EAkResult)ActionResult);
		});
	}
}

void FAkBankManager::BankLoadCallback(
	AkUInt32		in_bankID,
	const void *	in_pInMemoryBankPtr,
	AKRESULT		in_eLoadResult,
	AkMemPoolId		in_memPoolId,
	void *			in_pCookie
)
{
	if (in_pCookie)
	{
		IAkBankCallbackInfo* BankCbInfo = (IAkBankCallbackInfo*)in_pCookie;
		if (in_eLoadResult == AK_Success)
		{
			FScopeLock Lock(&GetInstance()->m_BankManagerCriticalSection);
			// Load worked; put the bank in the list.
			GetInstance()->AddLoadedBank(BankCbInfo->Bank);
		}

		BankCbInfo->HandleAction(in_bankID, in_pInMemoryBankPtr, in_eLoadResult, in_memPoolId);

		delete BankCbInfo;
	}
}

void FAkBankManager::BankUnloadCallback(
	AkUInt32		in_bankID,
	const void *	in_pInMemoryBankPtr,
	AKRESULT		in_eUnloadResult,
	AkMemPoolId		in_memPoolId,
	void *			in_pCookie
)
{
	if (in_pCookie)
	{
		IAkBankCallbackInfo* BankCbInfo = (IAkBankCallbackInfo*)in_pCookie;
		if (in_eUnloadResult == AK_Success)
		{
			FScopeLock Lock(&GetInstance()->m_BankManagerCriticalSection);
			// Load worked; put the bank in the list.
			GetInstance()->RemoveLoadedBank(BankCbInfo->Bank);
		}

		BankCbInfo->HandleAction(in_bankID, in_pInMemoryBankPtr, in_eUnloadResult, in_memPoolId);

		delete BankCbInfo;
	}
}

FAkBankManager::FAkBankManager()
{
	if (Instance != nullptr)
	{
		UE_LOG(LogInit, Error, TEXT("FAkBankManager has already been instantiated."));
	}

	Instance = this;
}

FAkBankManager::~FAkBankManager()
{
	if (Instance == this)
	{
		TSet<UAkAudioBank*> LoadedBanksCopy(m_LoadedBanks);
		for (auto* AudioBank : LoadedBanksCopy)
		{
			if (AudioBank != nullptr && AudioBank->IsValidLowLevel())
			{
				AudioBank->Unload();
			}
		}

		Instance = nullptr;
	}
}
