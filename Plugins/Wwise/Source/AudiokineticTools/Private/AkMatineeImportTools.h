// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#pragma once
#include "AkUEFeatures.h"
#include "MovieSceneAkAudioRTPCSection.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

class UInterpTrackAkAudioEvent;
class UInterpTrackAkAudioRTPC;

class UMovieSceneAkAudioEventTrack;
class UMovieSceneAkAudioRTPCTrack;

enum class ECopyInterpAkAudioResult
{
	NoChange,
	KeyModification,
	SectionAdded,
};

/**
 * Tools for AkAudio tracks
 */
class FAkMatineeImportTools
{
public:

	/** Attempts to retrieve a specific InterpTrackType from the MatineeCopyPasteBuffer. */
	template<typename InterpTrackType>
	static InterpTrackType* GetTrackFromMatineeCopyPasteBuffer()
	{
		for (auto CopyPasteObject : GUnrealEd->MatineeCopyPasteBuffer)
		{
			auto Track = Cast<InterpTrackType>(CopyPasteObject);
			if (Track != nullptr)
			{
				return Track;
			}
		}

		return nullptr;
	}

	/** Copies keys from a matinee AkAudioRTPC track to a sequencer AkAudioRTPC track. */
	static ECopyInterpAkAudioResult CopyInterpAkAudioRTPCTrack(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCTrack* AkAudioRTPCTrack);

	/** Copies keys from a matinee AkAudioEvent track to a sequencer AkAudioEvent track. */
	static ECopyInterpAkAudioResult CopyInterpAkAudioEventTrack(const UInterpTrackAkAudioEvent* MatineeAkAudioEventTrack, UMovieSceneAkAudioEventTrack* AkAudioEventTrack);

private:
#if UE_4_20_OR_LATER
	static void CopyInterpTrackToFloatChannel(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection* RtpcSection, ECopyInterpAkAudioResult& Result);
#else
	static void CopyInterpTrackToRichCurve(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection* RtpcSection, ECopyInterpAkAudioResult& Result);
#endif
};
