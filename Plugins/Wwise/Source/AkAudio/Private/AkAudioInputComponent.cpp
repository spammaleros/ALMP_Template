// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
AkAudioInputComponent.cpp:
=============================================================================*/

#include "AkAudioInputComponent.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAudioInputManager.h"

UAkAudioInputComponent::UAkAudioInputComponent(const class FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{}

int32 UAkAudioInputComponent::PostAssociatedAudioInputEvent()
{
	AkPlayingID PlayingID = FAkAudioInputManager::PostAudioInputEvent(GET_AK_EVENT_NAME(AkAudioEvent, EventName), this,
																	  FAkGlobalAudioInputDelegate::CreateUObject(this, &UAkAudioInputComponent::FillSamplesBuffer),
																	  FAkGlobalAudioFormatDelegate::CreateUObject(this, &UAkAudioInputComponent::GetChannelConfig));
	if (PlayingID != AK_INVALID_PLAYING_ID)
	{
		CurrentlyPlayingIDs.Add(PlayingID);
	}
	return PlayingID;
}

void UAkAudioInputComponent::PostUnregisterGameObject()
{
	auto Device = FAkAudioDevice::Get();
	if (Device != nullptr)
	{
		for (int i = 0; i < CurrentlyPlayingIDs.Num(); ++i)
		{
			Device->StopPlayingID(CurrentlyPlayingIDs[i]);
		}
	}
	CurrentlyPlayingIDs.Empty();
}