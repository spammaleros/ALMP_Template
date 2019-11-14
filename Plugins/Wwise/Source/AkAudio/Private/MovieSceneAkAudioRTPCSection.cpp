// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#include "MovieSceneAkAudioRTPCSection.h"
#include "AkAudioDevice.h"
#include "AkCustomVersion.h"
#include "MovieSceneAkAudioRTPCTemplate.h"

#if UE_4_20_OR_LATER
#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneChannelEditorData.h"
#endif
UMovieSceneAkAudioRTPCSection::UMovieSceneAkAudioRTPCSection(const FObjectInitializer& Init)
	: Super(Init)
{
#if UE_4_20_OR_LATER
	FMovieSceneChannelProxyData Channels;

#if WITH_EDITOR
	FMovieSceneChannelMetaData Metadata;
	Metadata.SetIdentifiers("RTPC", NSLOCTEXT("MovieSceneAkAudioRTPCSectionEditorData", "RTPC", "RTPC"));
	Channels.Add(RTPCChannel, Metadata, TMovieSceneExternalValue<float>());
#else
	Channels.Add(RTPCChannel);
#endif

	// Populate the channel proxy - if any of our channels were ever reallocated, we'd need to repopulate the proxy,
	// but since ours are all value member types, we only need to populate in the constructor
	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(MoveTemp(Channels));
#endif
}

FMovieSceneEvalTemplatePtr UMovieSceneAkAudioRTPCSection::GenerateTemplate() const
{
	return FMovieSceneAkAudioRTPCTemplate(*this);
}

#if UE_4_20_OR_LATER
float UMovieSceneAkAudioRTPCSection::GetStartTime() const
{
	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();
	return (float)FrameRate.AsSeconds(GetRange().GetLowerBoundValue());
}

float UMovieSceneAkAudioRTPCSection::GetEndTime() const
{
	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();
	return (float)FrameRate.AsSeconds(GetRange().GetUpperBoundValue());
}

void UMovieSceneAkAudioRTPCSection::PostLoad()
{
	Super::PostLoad();
	const int32 AkVersion = GetLinkerCustomVersion(FAkCustomVersion::GUID);

	if (AkVersion < FAkCustomVersion::NewRTPCTrackDataContainer)
	{

		if (FloatCurve.GetDefaultValue() != MAX_flt)
		{
			RTPCChannel.SetDefault(FloatCurve.GetDefaultValue());
		}

		RTPCChannel.PreInfinityExtrap = FloatCurve.PreInfinityExtrap;
		RTPCChannel.PostInfinityExtrap = FloatCurve.PostInfinityExtrap;

		TArray<FFrameNumber> Times;
		TArray<FMovieSceneFloatValue> Values;
		Times.Reserve(FloatCurve.GetNumKeys());
		Values.Reserve(FloatCurve.GetNumKeys());

		const FFrameRate LegacyFrameRate = GetLegacyConversionFrameRate();
		const float      Interval = LegacyFrameRate.AsInterval();

		int32 Index = 0;
		for (auto It = FloatCurve.GetKeyIterator(); It; ++It)
		{
			const FRichCurveKey& Key = *It;

			FFrameNumber KeyTime = UpgradeLegacyMovieSceneTime(nullptr, LegacyFrameRate, It->Time);

			FMovieSceneFloatValue NewValue;
			NewValue.Value = Key.Value;
			NewValue.InterpMode = Key.InterpMode;
			NewValue.TangentMode = Key.TangentMode;
			NewValue.Tangent.ArriveTangent = Key.ArriveTangent * Interval;
			NewValue.Tangent.LeaveTangent = Key.LeaveTangent  * Interval;
			ConvertInsertAndSort<FMovieSceneFloatValue>(Index++, KeyTime, NewValue, Times, Values);
		}

		RTPCChannel.Set(Times, Values);
		return;
	}

	FloatChannelSerializationHelper.ToFloatChannel(RTPCChannel);
}

void UMovieSceneAkAudioRTPCSection::Serialize(FArchive& Ar)
{
#if UE_4_20_OR_LATER
	FloatChannelSerializationHelper = RTPCChannel;
	Ar.UsingCustomVersion(FAkCustomVersion::GUID);
#endif
	Super::Serialize(Ar);
}

#else

void UMovieSceneAkAudioRTPCSection::MoveSection(float DeltaPosition, TSet<FKeyHandle>& KeyHandles)
{
	Super::MoveSection(DeltaPosition, KeyHandles);

	// Move the curve
	FloatCurve.ShiftCurve(DeltaPosition, KeyHandles);
}


void UMovieSceneAkAudioRTPCSection::DilateSection(float DilationFactor, float Origin, TSet<FKeyHandle>& KeyHandles)
{
	Super::DilateSection(DilationFactor, Origin, KeyHandles);

	FloatCurve.ScaleCurve(Origin, DilationFactor, KeyHandles);
}

void UMovieSceneAkAudioRTPCSection::GetKeyHandles(TSet<FKeyHandle>& OutKeyHandles, TRange<float> TimeRange) const
{
	if (!TimeRange.Overlaps(GetRange()))
	{
		return;
	}

	for (auto It(FloatCurve.GetKeyHandleIterator()); It; ++It)
	{
		float Time = FloatCurve.GetKeyTime(It.Key());
		if (TimeRange.Contains(Time))
		{
			OutKeyHandles.Add(It.Key());
		}
	}
}

TOptional<float> UMovieSceneAkAudioRTPCSection::GetKeyTime(FKeyHandle KeyHandle) const
{
	if (FloatCurve.IsKeyHandleValid(KeyHandle))
	{
		return TOptional<float>(FloatCurve.GetKeyTime(KeyHandle));
	}
	return TOptional<float>();
}

void UMovieSceneAkAudioRTPCSection::SetKeyTime(FKeyHandle KeyHandle, float Time)
{
	if (FloatCurve.IsKeyHandleValid(KeyHandle))
	{
		FloatCurve.SetKeyTime(KeyHandle, Time);
	}
}

void UMovieSceneAkAudioRTPCSection::AddKey(float Time, const float& Value, EMovieSceneKeyInterpolation KeyInterpolation)
{
	AddKeyToCurve(FloatCurve, Time, Value, KeyInterpolation);
}

bool UMovieSceneAkAudioRTPCSection::NewKeyIsNewData(float Time, const float& Value) const
{
	return FMath::IsNearlyEqual(FloatCurve.Eval(Time), Value) == false;
}

bool UMovieSceneAkAudioRTPCSection::HasKeys(const float& Value) const
{
	return FloatCurve.GetNumKeys() > 0;
}

void UMovieSceneAkAudioRTPCSection::SetDefault(const float& Value)
{
	SetCurveDefault(FloatCurve, Value);
}

void UMovieSceneAkAudioRTPCSection::ClearDefaults()
{
	FloatCurve.ClearDefaultValue();
}
#endif // UE_4_20_OR_LATER

#if WITH_EDITOR
void UMovieSceneAkAudioRTPCSection::PreEditChange(UProperty* PropertyAboutToChange)
{
	PreviousName = Name;
}

void UMovieSceneAkAudioRTPCSection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMovieSceneAkAudioRTPCSection, Name))
	{
		if (!IsRTPCNameValid())
		{
			Name = PreviousName;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool UMovieSceneAkAudioRTPCSection::IsRTPCNameValid()
{
	return !Name.IsEmpty();
}
#endif
