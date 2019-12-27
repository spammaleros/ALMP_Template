// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#pragma once

#include "AkInclude.h"
#include "AkAudioDevice.h"
#include "Templates/Function.h"

/*------------------------------------------------------------------------------------
AkAudioInput Delegates
------------------------------------------------------------------------------------*/

DECLARE_DELEGATE_RetVal_ThreeParams(bool, FAkGlobalAudioInputDelegate, uint32, uint32, float**);
DECLARE_DELEGATE_OneParam(FAkGlobalAudioFormatDelegate, AkAudioFormat&);

/*------------------------------------------------------------------------------------
FAkAudioInputManager
------------------------------------------------------------------------------------*/

class FAkAudioInputManager
{
public:

    /**
    * Post an event to ak soundengine
    *
    * @param Event Name of the event to post
    * @param Actor Actor on which to play the event
    * @param AudioSamplesCallback Callback that fills the audio samples buffer
    * @param AudioFormatCallback Callback that sets the audio format
    * @param GainCallback Callback that returns the 
	
	gain level for the audio input
    * @return ID assigned by ak soundengine
    */
    static AkPlayingID PostAudioInputEvent(
        class UAkAudioEvent * Event,
        AActor * Actor,
        FAkGlobalAudioInputDelegate AudioSamplesDelegate,
        FAkGlobalAudioFormatDelegate AudioFormatDelegate
    );

    /**
    * Post an event to ak soundengine by name
    *
    * @param EventName Name of the event to post
    * @param Actor Actor on which to play the event
    * @param AudioSamplesCallback Callback that fills the audio samples buffer
    * @param AudioFormatCallback Callback that sets the audio format
    * @param GainCallback Callback that returns the gain level for the audio input
    * @return ID assigned by ak soundengine
    */
    static AkPlayingID PostAudioInputEvent(
        const FString& EventName,
        AActor * Actor,
        FAkGlobalAudioInputDelegate AudioSamplesDelegate,
        FAkGlobalAudioFormatDelegate AudioFormatDelegate
    );

    /**
    * Post an event to ak soundengine by name
    *
    * @param EventName Name of the event to post
    * @param Component AkComponent on which to play the event
    * @param AudioSamplesCallback Callback that fills the audio samples buffer
    * @param AudioFormatCallback Callback that sets the audio format
    * @param GainCallback Callback that returns the gain level for the audio input
    * @return ID assigned by ak soundengine
    */
    static AkPlayingID PostAudioInputEvent(
        const FString& EventName,
        UAkComponent* Component,
        FAkGlobalAudioInputDelegate AudioSamplesDelegate,
        FAkGlobalAudioFormatDelegate AudioFormatDelegate
    );

};