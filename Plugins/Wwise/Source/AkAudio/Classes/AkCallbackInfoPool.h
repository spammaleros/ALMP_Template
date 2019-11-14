// Copyright (c) 2019 Audiokinetic Inc. / All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"

class UAkCallbackInfo;

class AkCallbackInfoPool final
{
public:
	~AkCallbackInfoPool();

	template<typename CallbackType>
	CallbackType* Acquire()
	{
		return static_cast<CallbackType*>(internalAcquire(CallbackType::StaticClass()));
	}

	void Release(UAkCallbackInfo* instance);

private:
	UAkCallbackInfo* internalAcquire(UClass* type);

private:
	FCriticalSection Mutex;
	TMap<UClass*, TArray<UAkCallbackInfo*>> Pool;
};
