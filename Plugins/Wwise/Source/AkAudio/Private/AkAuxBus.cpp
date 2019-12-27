// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkReverbVolume.cpp:
=============================================================================*/

#include "AkAuxBus.h"
#include "AkAudioDevice.h"

/*------------------------------------------------------------------------------------
	UAkAuxBus
------------------------------------------------------------------------------------*/

UAkAuxBus::UAkAuxBus(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AuxBusId = AK_INVALID_AUX_ID;
	}
	else
	{
		// Property initialization
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice)
		{
			AuxBusId = AkAudioDevice->GetIDFromString(GetName());
		}
		else
		{
			AuxBusId = AK_INVALID_AUX_ID;
		}
	}
}

