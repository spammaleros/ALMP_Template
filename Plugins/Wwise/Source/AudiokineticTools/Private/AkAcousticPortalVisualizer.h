// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
AkAcousticPortalVisualizer.h:
=============================================================================*/

#pragma once
#include "ComponentVisualizer.h"

class UAkPortalComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
