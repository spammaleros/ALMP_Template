// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
AkOcclusionObstructionService.cpp:
=============================================================================*/

#include "OcclusionObstructionService/AkPortalOcclusionObstructionService.h"
#include "OcclusionObstructionService/AkOcclusionObstructionService.h"
#include "AkAudioDevice.h"
#include "AkAcousticPortal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"


void AkPortalOcclusionObstructionService::Init(AAkAcousticPortal* in_portal, float in_refreshInterval)
{
	_Init(in_portal->GetWorld(), in_refreshInterval);
	AssociatedPortal = in_portal;
}
void AkPortalOcclusionObstructionService::SetOcclusionObstruction(AkGameObjectID ListenerId, float Value)
{
	AK::SpatialAudio::SetPortalObstructionAndOcclusion(AssociatedPortal->GetPortalID(), Value, 0.0f);
}
