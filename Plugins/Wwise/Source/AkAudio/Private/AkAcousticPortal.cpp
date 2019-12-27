// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AAkAcousticPortal.cpp:
=============================================================================*/

#include "AkAcousticPortal.h"
#include "AkAudioDevice.h"
#include "Components/BrushComponent.h"
#include "Model.h"
#include "EngineUtils.h"
#include "AkRoomComponent.h"
#include "AkComponent.h"

UAkPortalComponent::UAkPortalComponent(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{}

/*------------------------------------------------------------------------------------
	AAkAcousticPortal
------------------------------------------------------------------------------------*/

AAkAcousticPortal::AAkAcousticPortal(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// Property initialization
	static const FName CollisionProfileName(TEXT("OverlapAll"));
	GetBrushComponent()->SetCollisionProfileName(CollisionProfileName);

	bColored = true;
	BrushColor = FColor(255, 196, 137, 255);

	ObstructionRefreshInterval = 0.33f;

	InitialState = AkAcousticPortalState::Open;
	CurrentState = (int)InitialState;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

#ifdef WITH_EDITOR
	// Currently, the portal component only exists to support the portal visualizer.
	static const FName PortalComponentName = TEXT("PortalComponent");
	UAkPortalComponent* PortalComponent = ObjectInitializer.CreateDefaultSubobject<UAkPortalComponent>(this, PortalComponentName);
	PortalComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
#endif
}

void AAkAcousticPortal::OpenPortal()
{
	CurrentState++;

	if (CurrentState == 1)
	{
		FAkAudioDevice * Dev = FAkAudioDevice::Get();
		if (Dev != nullptr)
		{
			Dev->SetSpatialAudioPortal(this);
		}
	}
}

void AAkAcousticPortal::ClosePortal()
{
	CurrentState--;

	if (CurrentState == 0)
	{
		FAkAudioDevice * Dev = FAkAudioDevice::Get();
		if (Dev != nullptr)
		{
			Dev->SetSpatialAudioPortal(this);
		}
	}
}

AkAcousticPortalState AAkAcousticPortal::GetCurrentState() const
{
	return CurrentState > 0 ? AkAcousticPortalState::Open : AkAcousticPortalState::Closed;
}

void AAkAcousticPortal::PostRegisterAllComponents()
{
	CurrentState = (int)InitialState;

	Super::PostRegisterAllComponents();
}

void AAkAcousticPortal::BeginPlay()
{
	Super::BeginPlay();

	FAkAudioDevice * Dev = FAkAudioDevice::Get();
	if (Dev != nullptr)
	{
		FindConnectedRooms();
		Dev->SetSpatialAudioPortal(this);
		ObstructionService.Init(this, ObstructionRefreshInterval);
	}
}

void AAkAcousticPortal::BeginDestroy()
{
	Super::BeginDestroy();
	FAkAudioDevice * Dev = FAkAudioDevice::Get();
	if (Dev != nullptr)
	{
		Dev->RemoveSpatialAudioPortal(this);
	}
}

void AAkAcousticPortal::FindConnectedRooms() 
{
	FrontRoom = AkRoomID();
	BackRoom = AkRoomID();

	FAkAudioDevice * Dev = FAkAudioDevice::Get();
	UAkRoomComponent* pFront = nullptr, *pBack = nullptr;
	FindConnectedComponents(Dev->GetHighestPriorityRoomComponentMap(), pFront, pBack);
	
	if (pFront != NULL)
		FrontRoom = pFront->GetRoomID();
	if (pBack != NULL)
		BackRoom = pBack->GetRoomID();
}

FVector AAkAcousticPortal::GetExtent() const
{
	FVector scale = GetActorScale();

	UBrushComponent* pBrComp = GetBrushComponent();
	if (pBrComp && pBrComp->Brush && pBrComp->Brush->Points.Num() == 8)
	{
		// Assuming that the box is indeed a box with 8 corners, we set the scale accordingly.
		scale = scale * (pBrComp->Brush->Points[4] - pBrComp->Brush->Points[0]) / 2.f;
	}

	return scale;
}

template <typename tComponent>
void AAkAcousticPortal::FindConnectedComponents(TMap<UWorld*, tComponent*>& HighestPriorityComponentMap, tComponent*& out_pFront, tComponent*& out_pBack) const
{
	out_pFront = nullptr;
	out_pBack = nullptr;

	FAkAudioDevice* pAudioDevice = FAkAudioDevice::Get();
	if (pAudioDevice != nullptr)
	{
		UBrushComponent* pBrComp = GetBrushComponent();
		if (pBrComp && pBrComp->Brush)
		{
			float y = 100.f;
			if (pBrComp->Brush->Points.Num() > 4)
				y = fabs(pBrComp->Brush->Points[4].Y);

			FVector frontVector(0.f, y, 0.f);

			FTransform toWorld = GetTransform();

			FVector frontPoint = toWorld.TransformPosition(frontVector);
			FVector backPoint = toWorld.TransformPosition(-1*frontVector);

			TArray<tComponent*> front = pAudioDevice->FindPrioritizedComponentsAtLocation(frontPoint, GetWorld(), HighestPriorityComponentMap, 1);
			if (front.Num() > 0)
				out_pFront = front[0];

			TArray<tComponent*> back = pAudioDevice->FindPrioritizedComponentsAtLocation(backPoint, GetWorld(), HighestPriorityComponentMap, 1);
			if (back.Num() > 0)
				out_pBack = back[0];

		}
	}
}

void AAkAcousticPortal::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	if (GetCurrentState() == AkAcousticPortalState::Open)
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice)
		{
			UAkComponent* Listener = AkAudioDevice->GetSpatialAudioListener();
			if (Listener != nullptr)
			{
				AkRoomID listenerRoom = Listener->GetSpatialAudioRoom();
				UAkComponentSet set;
				set.Add(Listener);
				ObstructionService.Tick(set, GetActorLocation(), this, listenerRoom, ObstructionCollisionChannel, DeltaTime, ObstructionRefreshInterval);
			}
		}
	}
}