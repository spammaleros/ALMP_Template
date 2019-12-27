// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkSpatialAudioVolume.cpp:
=============================================================================*/

#include "AkSpatialAudioVolume.h"
#include "AkAudioDevice.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkSurfaceReflectorSetComponent.h"
#include "Components/BrushComponent.h"
#include "Model.h"
/*------------------------------------------------------------------------------------
	AAkSpatialAudioVolume
------------------------------------------------------------------------------------*/

AAkSpatialAudioVolume::AAkSpatialAudioVolume(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// Property initialization
	static const FName CollisionProfileName(TEXT("OverlapAll"));
	UBrushComponent* BrushComp = GetBrushComponent();
	if (BrushComp)
	{
		BrushComp->SetCollisionProfileName(CollisionProfileName);
	}

	static const FName SurfReflSetName = TEXT("SurfaceReflector");
	SurfaceReflectorSet = ObjectInitializer.CreateDefaultSubobject<UAkSurfaceReflectorSetComponent>(this, SurfReflSetName);
	SurfaceReflectorSet->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	static const FName LateReverbame = TEXT("LateReverb");
	LateReverb = ObjectInitializer.CreateDefaultSubobject<UAkLateReverbComponent>(this, LateReverbame);
	LateReverb->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	static const FName RoomName = TEXT("Room");
	Room = ObjectInitializer.CreateDefaultSubobject<UAkRoomComponent>(this, RoomName);
	Room->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	bColored = true;
	BrushColor = FColor(109, 187, 255, 255);
}

