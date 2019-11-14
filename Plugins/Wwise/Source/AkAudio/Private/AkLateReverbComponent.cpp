// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkLateReverbComponent.cpp:
=============================================================================*/

#include "AkLateReverbComponent.h"
#include "AkAudioDevice.h"
#include "AkAuxBus.h"
#include "AkRoomComponent.h"
#include "Components/BrushComponent.h"
#include "Model.h"
#include "GameFramework/Volume.h"

/*------------------------------------------------------------------------------------
	UAkLateReverbComponent
------------------------------------------------------------------------------------*/

UAkLateReverbComponent::UAkLateReverbComponent(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	ParentVolume = nullptr;

	// Property initialization
	SendLevel = 1.0f;
	FadeRate = 0.5f;
	Priority = 1.0f;

	NextLowerPriorityComponent = nullptr;

	bEnable = true;
	bWantsInitializeComponent = true;
}

bool UAkLateReverbComponent::HasEffectOnLocation(const FVector& Location) const
{
	// Need to add a small radius, because on the Mac, EncompassesPoint returns false if
	// Location is exactly equal to the Volume's location
	static float RADIUS = 0.01f;
	return LateReverbIsActive() && ParentVolume->EncompassesPoint(Location, RADIUS);
}

uint32 UAkLateReverbComponent::GetAuxBusId() const
{
	if (AuxBus)
	{
		return AuxBus->GetAuxBusId();
	}
	else
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice)
		{
			return AkAudioDevice->GetIDFromString(AuxBusName);
		}
		return AK_INVALID_UNIQUE_ID;
	}
}

void UAkLateReverbComponent::InitializeParentVolume()
{
	ParentVolume = Cast<AVolume>(GetOwner());
	if (!ParentVolume)
	{
		bEnable = false;
		UE_LOG(LogAkAudio, Error, TEXT("UAkLateReverbComponent requires to be attached to an actor inheriting from AVolume."));
	}
}

#if WITH_EDITOR
void UAkLateReverbComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	InitializeParentVolume();
}
#endif // WITH_EDITOR

void UAkLateReverbComponent::PostLoad()
{
	Super::PostLoad();
	InitializeParentVolume();
}

void UAkLateReverbComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	UAkRoomComponent* pRoomCmpt = nullptr;
	if (ParentVolume)
	{
		pRoomCmpt = (UAkRoomComponent*)ParentVolume->GetComponentByClass(UAkRoomComponent::StaticClass());
	}

	if (!pRoomCmpt || !pRoomCmpt->RoomIsActive())
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice && LateReverbIsActive())
		{
			AkAudioDevice->AddLateReverbComponentToPrioritizedList(this);
		}
	}
}

void UAkLateReverbComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice && LateReverbIsActive())
	{
		AkAudioDevice->RemoveLateReverbComponentFromPrioritizedList(this);
	}
}

