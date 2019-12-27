// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
AkPortalOcclusionObstructionService.h:
=============================================================================*/

#pragma once

#include "AkInclude.h"
#include "AkAudioDevice.h"
#include "WorldCollision.h"
#include "HAL/ThreadSafeBool.h"
#include "OcclusionObstructionService/AkOcclusionObstructionService.h"

class AActor;
class AAkAcousticPortal;

class AkPortalOcclusionObstructionService : public AkOcclusionObstructionService
{
public:
	void Init(AAkAcousticPortal* in_portalId, float in_refreshInterval);
	
	virtual void SetOcclusionObstruction(AkGameObjectID ListenerID, float Value);

	virtual ~AkPortalOcclusionObstructionService() {}

private:
	AAkAcousticPortal * AssociatedPortal;
};
