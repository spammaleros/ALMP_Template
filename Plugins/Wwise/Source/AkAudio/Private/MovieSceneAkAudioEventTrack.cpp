// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#include "MovieSceneAkAudioEventTrack.h"
#include "AkAudioDevice.h"

#include "IMovieScenePlayer.h"
#include "MovieScene.h"
#include "MovieSceneAkAudioEventSection.h"

#include "MovieSceneAkAudioEventTemplate.h"

FMovieSceneEvalTemplatePtr UMovieSceneAkAudioEventTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return InSection.GenerateTemplate();
}

UMovieSceneSection* UMovieSceneAkAudioEventTrack::CreateNewSection()
{
	return NewObject<UMovieSceneSection>(this, UMovieSceneAkAudioEventSection::StaticClass(), NAME_None, RF_Transactional);
}

bool UMovieSceneAkAudioEventTrack::AddNewEvent(TimeUnit Time, UAkAudioEvent* Event, const FString& EventName)
{
    UMovieSceneAkAudioEventSection* NewSection = NewObject<UMovieSceneAkAudioEventSection>(this);
	ensure(NewSection);

	NewSection->SetEvent(Event, EventName);
	const auto Duration = NewSection->GetMaxEventDuration();
#if UE_4_20_OR_LATER
	NewSection->InitialPlacement(GetAllSections(), Time, Duration, SupportsMultipleRows());
#else
	NewSection->InitialPlacement(GetAllSections(), Time, Time + Duration, SupportsMultipleRows());
#endif
	AddSection(*NewSection);

	return true;
}

void UMovieSceneAkAudioEventTrack::WorkUnitChangesDetectedFromSection(UMovieSceneAkAudioEventSection* in_pSection)
{
    for (auto Section : Sections)
    {
        if (UMovieSceneAkAudioEventSection* AkSection = Cast<UMovieSceneAkAudioEventSection>(Section))
        {
            if (AkSection != in_pSection)
            {
                AkSection->CheckForWorkunitChanges();
            }
        }
    }
}

#if WITH_EDITORONLY_DATA
FText UMovieSceneAkAudioEventTrack::GetDisplayName() const
{
	return NSLOCTEXT("MovieSceneAkAudioEventTrack", "TrackName", "AkAudioEvents");
}
#endif

FName UMovieSceneAkAudioEventTrack::GetTrackName() const
{
	static FName TrackName("AkAudioEvents");
	return TrackName;
}
