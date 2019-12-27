// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
AkComponentOcclusionObstructionService.h:
=============================================================================*/

#pragma once

#include "AkInclude.h"
#include "AkAudioDevice.h"
#include "WorldCollision.h"
#include "HAL/ThreadSafeBool.h"
#include "OcclusionObstructionService/AkOcclusionObstructionService.h"

class UAkComponent;

class AkComponentOcclusionObstructionService : public AkOcclusionObstructionService
{
public:
	void Init(UAkComponent* in_akComponent, float in_refreshInterval);

	virtual void SetOcclusionObstruction(AkGameObjectID ListenerID, float Value);

	virtual ~AkComponentOcclusionObstructionService() {}

private:
	UAkComponent * AssociatedComponent;
};