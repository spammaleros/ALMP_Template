// Fill out your copyright notice in the Description page of Project Settings.

#include "AkSpotReflector.h"
#include "AkAudioDevice.h"
#include "AkAuxBus.h"
#include "AkRoomComponent.h"
#include "Engine/Texture2D.h"
#include "Components/BillboardComponent.h"

#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

// Sets default values
AAkSpotReflector::AAkSpotReflector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AcousticTexture(NULL)
    , DistanceScalingFactor(2.f)
    , Level(1.f)
	, m_uAuxBusID(AK_INVALID_AUX_ID)
{
	static const FName ComponentName = TEXT("SpotReclectorRootComponent");
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, ComponentName);

#if WITH_EDITORONLY_DATA
	static const FName SpriteComponentName = TEXT("Sprite");
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(SpriteComponentName);
	if (SpriteComponent) 
	{
		SpriteComponent->SetSprite(LoadObject<UTexture2D>(NULL, TEXT("/Wwise/S_AkSpotReflector.S_AkSpotReflector")));
		SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
		SpriteComponent->SetupAttachment(RootComponent);
	}
#endif

	// AActor properties 
	bHidden = true;
	bCanBeDamaged = false;
}

// Called when the game starts or when spawned
void AAkSpotReflector::BeginPlay()
{
	Super::BeginPlay();

#if UE_4_17_OR_LATER
	const auto& RootTransform = RootComponent->GetComponentTransform();
#else
	const auto& RootTransform = RootComponent->ComponentToWorld;
#endif // UE_4_17_OR_LATER
	
	FAkAudioDevice* pDev = FAkAudioDevice::Get();
	if(!pDev)
		return;

	AkImageSourceSettings sourceInfo(
		FAkAudioDevice::FVectorToAKVector(RootTransform.GetTranslation()),
		DistanceScalingFactor, Level);

#if WITH_EDITOR
	sourceInfo.SetName(TCHAR_TO_ANSI(*GetActorLabel()));
#endif // WITH_EDITOR

	if (AcousticTexture)
	{
		sourceInfo.SetOneTexture(FAkAudioDevice::Get()->GetIDFromString(AcousticTexture->GetName()));
	}

	if (AuxBus)
	{
		m_uAuxBusID = AuxBus->GetAuxBusId();
	}
	else
	{
		if (!AuxBusName.IsEmpty())
			m_uAuxBusID = pDev->GetIDFromString(AuxBusName);
		else
			m_uAuxBusID = AK_INVALID_UNIQUE_ID;
	}

	AkRoomID roomID;
	TArray<UAkRoomComponent*> AkRooms = pDev->FindRoomComponentsAtLocation(RootTransform.GetTranslation(), GetWorld(), 1);
	if (AkRooms.Num() > 0)
		roomID = AkRooms[0]->GetRoomID();

	pDev->SetImageSource(this, sourceInfo, m_uAuxBusID, roomID);
}

void AAkSpotReflector::BeginDestroy()
{
	Super::BeginDestroy();
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (!IsRunningCommandlet() && AkAudioDevice)
	{
		AkAudioDevice->RemoveImageSource(this, m_uAuxBusID);
	}
}