// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkEvent.h:
=============================================================================*/
#pragma once

#include "Engine/GameEngine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "AkAcousticTexture.generated.h"

/*------------------------------------------------------------------------------------
	UAkAudioEvent
------------------------------------------------------------------------------------*/
UCLASS(meta=(BlueprintSpawnableComponent))
class AKAUDIO_API UAkAcousticTexture : public UPhysicalMaterial
{
	GENERATED_UCLASS_BODY()

public:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="AkTexture")
	FLinearColor	EditColor;
#endif
};
