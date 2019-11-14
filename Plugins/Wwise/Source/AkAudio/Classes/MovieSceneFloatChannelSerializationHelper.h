// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#pragma once

#include "AkInclude.h"
#include "MovieSceneSection.h"
#if UE_4_20_OR_LATER
#include "Channels/MovieSceneFloatChannel.h"
#endif
#include "MovieSceneFloatChannelSerializationHelper.generated.h"

USTRUCT()
struct FMovieSceneTangentDataSerializationHelper
{
	GENERATED_BODY()

	FMovieSceneTangentDataSerializationHelper() {}

#if UE_4_20_OR_LATER
	FMovieSceneTangentDataSerializationHelper(const FMovieSceneTangentData TangentData)
		: ArriveTangent(TangentData.ArriveTangent)
		, LeaveTangent(TangentData.LeaveTangent)
		, TangentWeightMode(TangentData.TangentWeightMode)
		, ArriveTangentWeight(TangentData.ArriveTangentWeight)
		, LeaveTangentWeight(TangentData.LeaveTangentWeight)
	{
	}
#endif

	UPROPERTY()
	float ArriveTangent = 0.f;

	UPROPERTY()
	float LeaveTangent = 0.f;

	UPROPERTY()
	TEnumAsByte<ERichCurveTangentWeightMode> TangentWeightMode = ERichCurveTangentWeightMode();

	UPROPERTY()
	float ArriveTangentWeight = 0.f;

	UPROPERTY()
	float LeaveTangentWeight = 0.f;
};

USTRUCT()
struct FMovieSceneFloatValueSerializationHelper
{
	GENERATED_BODY()

	FMovieSceneFloatValueSerializationHelper() {}

#if UE_4_20_OR_LATER
	FMovieSceneFloatValueSerializationHelper(FMovieSceneFloatValue FloatValue)
		: Value(FloatValue.Value)
		, InterpMode(FloatValue.InterpMode)
		, TangentMode(FloatValue.TangentMode)
		, Tangent(FloatValue.Tangent)
	{
	}

	FMovieSceneFloatValue ToFloatValue()
	{
		FMovieSceneFloatValue ValueToReturn(Value);
		ValueToReturn.InterpMode = InterpMode;
		ValueToReturn.TangentMode = TangentMode;
		ValueToReturn.Tangent.ArriveTangent = Tangent.ArriveTangent;
		ValueToReturn.Tangent.ArriveTangentWeight = Tangent.ArriveTangentWeight;
		ValueToReturn.Tangent.LeaveTangent = Tangent.LeaveTangent;
		ValueToReturn.Tangent.LeaveTangentWeight = Tangent.LeaveTangentWeight;
		ValueToReturn.Tangent.TangentWeightMode = Tangent.TangentWeightMode;
		return ValueToReturn;
	}
#endif

	UPROPERTY()
	float Value = 0.f;

	UPROPERTY()
	TEnumAsByte<ERichCurveInterpMode> InterpMode = ERichCurveInterpMode();

	UPROPERTY()
	TEnumAsByte<ERichCurveTangentMode> TangentMode = ERichCurveTangentMode();

	UPROPERTY()
	FMovieSceneTangentDataSerializationHelper Tangent;
};

USTRUCT()
struct FMovieSceneFloatChannelSerializationHelper
{
	GENERATED_BODY()

	FMovieSceneFloatChannelSerializationHelper() {}
#if UE_4_20_OR_LATER
	const FMovieSceneFloatChannelSerializationHelper& operator= (const FMovieSceneFloatChannel& FloatChannel)
	{
		Times.Reset();
		Values.Reset();
		
		PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
		PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

		auto FloatChannelTimes = FloatChannel.GetTimes();
		auto FloatChannelValues = FloatChannel.GetValues();
		for (int i = 0; i < FloatChannelTimes.Num(); i++)
		{
			Times.Add(FloatChannelTimes[i].Value);
			Values.Add(FMovieSceneFloatValueSerializationHelper(FloatChannelValues[i]));
		}

		bHasDefaultValue = FloatChannel.GetDefault().IsSet();
		if (bHasDefaultValue)
		{
			DefaultValue = FloatChannel.GetDefault().GetValue();
		}
		return *this;
	}

	void ToFloatChannel(FMovieSceneFloatChannel& FloatChannel)
	{
		FloatChannel.Reset();
		FloatChannel.PreInfinityExtrap = PreInfinityExtrap;
		FloatChannel.PostInfinityExtrap = PostInfinityExtrap;

		TArray<FFrameNumber> FloatChannelTimes;
		TArray<FMovieSceneFloatValue> FloatChannelValues;
		for (int i = 0; i < Times.Num(); i++)
		{
			FloatChannelTimes.Add(Times[i]);
			FloatChannelValues.Add(Values[i].ToFloatValue());
		}

		FloatChannel.Set(FloatChannelTimes, FloatChannelValues);

		if (bHasDefaultValue)
		{
			FloatChannel.SetDefault(DefaultValue);
		}
	}
#endif

	UPROPERTY()
	TEnumAsByte<ERichCurveExtrapolation> PreInfinityExtrap = ERichCurveExtrapolation();

	UPROPERTY()
	TEnumAsByte<ERichCurveExtrapolation> PostInfinityExtrap = ERichCurveExtrapolation();

	UPROPERTY()
	TArray<int32> Times;

	UPROPERTY()
	TArray<FMovieSceneFloatValueSerializationHelper> Values;

	UPROPERTY()
	float DefaultValue = 0.f;

	UPROPERTY()
	bool bHasDefaultValue = false;
};