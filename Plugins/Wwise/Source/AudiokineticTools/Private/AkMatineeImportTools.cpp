// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#include "AkMatineeImportTools.h"
#include "AkAudioDevice.h"
#include "InterpTrackAkAudioRTPC.h"
#include "InterpTrackAkAudioEvent.h"
#include "MovieSceneAkAudioRTPCSection.h"
#include "MovieSceneAkAudioRTPCTrack.h"
#include "MovieSceneAkAudioEventSection.h"
#include "MovieSceneAkAudioEventTrack.h"
#include "AkUEFeatures.h"
#include "MatineeImportTools.h"

#include "MovieSceneCommonHelpers.h"
#include "ScopedTransaction.h"


#define LOCTEXT_NAMESPACE "AkAudio"


/** Copies keys from a matinee AkAudioRTPC track to a sequencer AkAudioRTPC track. */
ECopyInterpAkAudioResult FAkMatineeImportTools::CopyInterpAkAudioRTPCTrack(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCTrack* AkAudioRTPCTrack)
{
	ECopyInterpAkAudioResult Result = ECopyInterpAkAudioResult::NoChange;
	const FScopedTransaction Transaction(LOCTEXT("PasteMatineeAkAudioRTPCTrack", "Paste Matinee AkAudioRtpc Track"));

	AkAudioRTPCTrack->Modify();

	// Get the name of the RTPC used on the Matinee track
	const FString& RTPCName = MatineeAkAudioRTPCTrack->Param;

	float MatineeTime = MatineeAkAudioRTPCTrack->GetKeyframeTime(0);
#if UE_4_20_OR_LATER
	FFrameRate FrameRate = AkAudioRTPCTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameNumber KeyTime = FrameRate.AsFrameNumber(MatineeTime);
#else
	float KeyTime = MatineeTime;
#endif

	UMovieSceneSection* Section = MovieSceneHelpers::FindSectionAtTime(AkAudioRTPCTrack->GetAllSections(), KeyTime);

	if (Section == nullptr)
	{
		Section = AkAudioRTPCTrack->CreateNewSection();
		AkAudioRTPCTrack->AddSection(*Section);
#if UE_4_20_OR_LATER
		Section->SetRange(TRange<FFrameNumber>::All());
#else
		Section->SetIsInfinite(true);
#endif
		Result = ECopyInterpAkAudioResult::SectionAdded;
	}

	// if this cast fails, UMovieSceneAkAudioRTPCTrack must not be creating UMovieSceneAkAudioRTPCSection's - BIG FAIL
	UMovieSceneAkAudioRTPCSection* RtpcSection = Cast<UMovieSceneAkAudioRTPCSection>(Section);
	RtpcSection->SetRTPCName(RTPCName);

	if (Section->TryModify())
	{
#if UE_4_20_OR_LATER
		CopyInterpTrackToFloatChannel(MatineeAkAudioRTPCTrack, RtpcSection, Result);
#else
		CopyInterpTrackToRichCurve(MatineeAkAudioRTPCTrack, RtpcSection, Result);
#endif
	}

	return Result;
}
#if UE_4_20_OR_LATER
void FAkMatineeImportTools::CopyInterpTrackToFloatChannel(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection* RtpcSection, ECopyInterpAkAudioResult& Result)
{
	float SectionMin = RtpcSection->GetStartTime();
	float SectionMax = RtpcSection->GetEndTime();

	FMovieSceneFloatChannel FloatChannel = RtpcSection->GetChannel();
	auto& Points = MatineeAkAudioRTPCTrack->FloatTrack.Points;

	FFrameRate FrameRate = RtpcSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameNumber FirstKeyTime = (MatineeAkAudioRTPCTrack->GetKeyframeTime(0) * FrameRate).RoundToFrame();;

	if (ECopyInterpAkAudioResult::NoChange == Result && Points.Num() > 0)
		Result = ECopyInterpAkAudioResult::KeyModification;

	TMovieSceneChannelData<FMovieSceneFloatValue> ChannelData = FloatChannel.GetData();
	TRange<FFrameNumber> KeyRange = TRange<FFrameNumber>::Empty();
	for (const auto& Point : Points)
	{
		FFrameNumber KeyTime = (Point.InVal * FrameRate).RoundToFrame();

#if UE_4_21_OR_LATER
		FMatineeImportTools::SetOrAddKey(ChannelData, KeyTime, Point.OutVal, Point.ArriveTangent, Point.LeaveTangent, Point.InterpMode, FrameRate);
#else
		FMatineeImportTools::SetOrAddKey(ChannelData, KeyTime, Point.OutVal, Point.ArriveTangent, Point.LeaveTangent, Point.InterpMode);
#endif

		KeyRange = TRange<FFrameNumber>::Hull(KeyRange, TRange<FFrameNumber>(KeyTime));
	}

	FKeyDataOptimizationParams Params;
	Params.bAutoSetInterpolation = true;

	MovieScene::Optimize(&FloatChannel, Params);

	if (!KeyRange.IsEmpty())
	{
		RtpcSection->SetRange(KeyRange);
	}
}
#else
void FAkMatineeImportTools::CopyInterpTrackToRichCurve(const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection* RtpcSection, ECopyInterpAkAudioResult& Result)
{
	float SectionMin = RtpcSection->GetStartTime();
	float SectionMax = RtpcSection->GetEndTime();

	FRichCurve& FloatCurve = RtpcSection->GetFloatCurve();
	auto& Points = MatineeAkAudioRTPCTrack->FloatTrack.Points;

	if (ECopyInterpAkAudioResult::NoChange == Result && Points.Num() > 0)
		Result = ECopyInterpAkAudioResult::KeyModification;

	for (const auto& Point : Points)
	{
		auto KeyHandle = FloatCurve.FindKey(Point.InVal);

		if (!FloatCurve.IsKeyHandleValid(KeyHandle))
		{
			KeyHandle = FloatCurve.AddKey(Point.InVal, Point.OutVal, false);
		}

		auto& Key = FloatCurve.GetKey(KeyHandle);
		Key.ArriveTangent = Point.ArriveTangent;
		Key.LeaveTangent = Point.LeaveTangent;
		Key.InterpMode = FMatineeImportTools::MatineeInterpolationToRichCurveInterpolation(Point.InterpMode);
		Key.TangentMode = FMatineeImportTools::MatineeInterpolationToRichCurveTangent(Point.InterpMode);

		SectionMin = FMath::Min(SectionMin, Point.InVal);
		SectionMax = FMath::Max(SectionMax, Point.InVal);
	}

	FloatCurve.RemoveRedundantKeys(KINDA_SMALL_NUMBER);
	FloatCurve.AutoSetTangents();

	RtpcSection->SetStartTime(SectionMin);
	RtpcSection->SetEndTime(SectionMax);
}
#endif

/** Copies keys from a matinee AkAudioEvent track to a sequencer AkAudioEvent track. */
ECopyInterpAkAudioResult FAkMatineeImportTools::CopyInterpAkAudioEventTrack(const UInterpTrackAkAudioEvent* MatineeAkAudioEventTrack, UMovieSceneAkAudioEventTrack* AkAudioEventTrack)
{
	ECopyInterpAkAudioResult Result = ECopyInterpAkAudioResult::NoChange;
	const FScopedTransaction Transaction(LOCTEXT("PasteMatineeAkAudioEventTrack", "Paste Matinee AkAudioEvent Track"));

	AkAudioEventTrack->Modify();

	auto& Events = MatineeAkAudioEventTrack->Events;
	for (const auto& Event : Events)
	{
#if UE_4_20_OR_LATER
		FFrameRate FrameRate = AkAudioEventTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
		FFrameNumber EventTime = FrameRate.AsFrameNumber(Event.Time);
#else
		float EventTime = Event.Time;
#endif
		if (AkAudioEventTrack->AddNewEvent(EventTime, Event.AkAudioEvent, Event.EventName))
		{
			Result = ECopyInterpAkAudioResult::SectionAdded;
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

