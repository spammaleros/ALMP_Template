// Copyright (c) 2019 Audiokinetic Inc. / All Rights Reserved
#include "AkCallbackInfoPool.h"
#include "AkGameplayTypes.h"

AkCallbackInfoPool::~AkCallbackInfoPool()
{
	for (auto& poolEntry : Pool)
	{
		for (auto instance : poolEntry.Value)
		{
			if (!instance)
				continue;

			instance->ClearFlags(RF_Public | RF_Standalone);
			instance->SetFlags(RF_Transient);
			instance->RemoveFromRoot();
		}
	}
}

UAkCallbackInfo* AkCallbackInfoPool::internalAcquire(UClass* type)
{
	{
		FScopeLock autoLock(&Mutex);
		auto& poolArray = Pool.FindOrAdd(type);
		if (poolArray.Num() > 0)
			return poolArray.Pop();
	}

	auto instance = NewObject<UAkCallbackInfo>(GetTransientPackage(), type, NAME_None, RF_Public | RF_Standalone);
	instance->AddToRoot();
	return instance;
}

void AkCallbackInfoPool::Release(UAkCallbackInfo* instance)
{
	FScopeLock autoLock(&Mutex);
	if (Pool.Contains(instance->GetClass()))
	{
		Pool[instance->GetClass()].Push(instance);
	}
}
