// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
IAkOcclusionObstructionService.cpp:
=============================================================================*/

#include "OcclusionObstructionService/AkComponentOcclusionObstructionService.h"
#include "OcclusionObstructionService/AkOcclusionObstructionService.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "Engine/World.h"

void AkComponentOcclusionObstructionService::Init(UAkComponent* in_akComponent, float in_refreshInterval)
{
	_Init(in_akComponent->GetWorld(), in_refreshInterval);
	AssociatedComponent = in_akComponent;
}

void AkComponentOcclusionObstructionService::SetOcclusionObstruction(AkGameObjectID ListenerId, float Value)
{
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice)
	{
		AkGameObjectID gameObjId = AssociatedComponent->GetAkGameObjectID();
		bool bUsingRooms = AkAudioDevice->UsingSpatialAudioRooms(AssociatedComponent->GetWorld());

		if (!AssociatedComponent->bUseSpatialAudio)
		{
			// If not using spatial audio, you're not using rooms (which are a Spatial Audio feature).
			// Therefore, we will always set occlusion.
			AK::SoundEngine::SetObjectObstructionAndOcclusion(gameObjId, ListenerId, 0.0f, Value);
		}
		else if (bUsingRooms)
		{
			AK::SpatialAudio::SetEmitterObstructionAndOcclusion(gameObjId, Value, 0.0f);
		}
		else // if (!bUsingRooms)
		{
			AK::SpatialAudio::SetEmitterObstructionAndOcclusion(gameObjId, 0.0f, Value);
		}
	}
}