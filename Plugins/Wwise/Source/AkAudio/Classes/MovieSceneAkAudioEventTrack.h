// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#pragma once

#include "MovieSceneAkTrack.h"
#include "IMovieScenePlayer.h"
#include "AkInclude.h"
#include "AkAudioEvent.h"
#include "MovieSceneAkAudioEventTrack.generated.h"

class UMovieSceneAkAudioEventSection;

UCLASS(MinimalAPI)
class UMovieSceneAkAudioEventTrack : public UMovieSceneAkTrack
{
	GENERATED_BODY()

public:
	UMovieSceneAkAudioEventTrack()
	{
#if WITH_EDITORONLY_DATA
		SetColorTint(FColor(0, 156, 255, 65));
#endif
	}

#if UE_4_20_OR_LATER
	typedef FFrameNumber TimeUnit;
#else
	typedef float TimeUnit;
#endif


	AKAUDIO_API virtual UMovieSceneSection* CreateNewSection() override;
	virtual bool SupportsMultipleRows() const override { return true; }

	AKAUDIO_API virtual FName GetTrackName() const override;

#if WITH_EDITORONLY_DATA
	AKAUDIO_API virtual FText GetDisplayName() const override;
#endif

	AKAUDIO_API bool AddNewEvent(TimeUnit Time, UAkAudioEvent* Event, const FString& EventName = FString());

    void WorkUnitChangesDetectedFromSection(UMovieSceneAkAudioEventSection* in_pSection);

protected:
	AKAUDIO_API virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;
};
