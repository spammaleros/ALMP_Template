// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#pragma once

#include "AkInclude.h"
#include "AkAudioEvent.h"
#include "Curves/RichCurve.h"
#include "MovieSceneSection.h"
#include "MovieSceneFloatChannelSerializationHelper.h"

#if UE_4_20_OR_LATER
#include "Channels/MovieSceneFloatChannel.h"
#else
#include "IKeyframeSection.h"
#endif

#include "MovieSceneAkAudioRTPCSection.generated.h"

/**
* A single floating point section
*/
UCLASS()
class AKAUDIO_API UMovieSceneAkAudioRTPCSection
	: public UMovieSceneSection
#if !UE_4_20_OR_LATER
	, public IKeyframeSection<float>
#endif
{
	GENERATED_BODY()
	UMovieSceneAkAudioRTPCSection(const FObjectInitializer& Init);
#if UE_4_20_OR_LATER
	virtual void PostLoad() override;
#endif
	virtual void Serialize(FArchive& Ar) override;

public:
#if UE_4_20_OR_LATER
	FMovieSceneFloatChannel GetChannel() const {	return RTPCChannel;	}
	float GetStartTime() const;
	float GetEndTime() const;
#else
	/**
	* @return The float curve on this section
	*/
	FRichCurve& GetFloatCurve() { return FloatCurve; }
	const FRichCurve& GetFloatCurve() const { return FloatCurve; }

	//~ IKeyframeSection interface

	void AddKey(float Time, const float& Value, EMovieSceneKeyInterpolation KeyInterpolation) override;
	bool NewKeyIsNewData(float Time, const float& Value) const override;
	bool HasKeys(const float& Value) const override;
	void SetDefault(const float& Value) override;
	virtual void ClearDefaults() override;


	//~ UMovieSceneSection interface

	virtual void MoveSection(float DeltaPosition, TSet<FKeyHandle>& KeyHandles) override;
	virtual void DilateSection(float DilationFactor, float Origin, TSet<FKeyHandle>& KeyHandles) override;
	virtual void GetKeyHandles(TSet<FKeyHandle>& OutKeyHandles, TRange<float> TimeRange) const override;
	virtual TOptional<float> GetKeyTime(FKeyHandle KeyHandle) const override;
	virtual void SetKeyTime(FKeyHandle KeyHandle, float Time) override;
#endif

	virtual FMovieSceneEvalTemplatePtr GenerateTemplate() const override;

	/** @return the name of the RTPC being modified by this track */
	const FString& GetRTPCName() const { return Name; }

	/**
	* Sets the name of the RTPC being modified by this track
	*
	* @param InRTPCName The RTPC being modified
	*/
	void SetRTPCName(const FString& InRTPCName) { Name = InRTPCName; }


#if WITH_EDITOR
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	/** Name of the RTPC to modify. */
	UPROPERTY(EditAnywhere, Category = "AkAudioRTPC", meta = (NoResetToDefault))
	FString Name;

	/** Curve data */
	UPROPERTY()
	FRichCurve FloatCurve;

	UPROPERTY()
	FMovieSceneFloatChannelSerializationHelper FloatChannelSerializationHelper;

#if UE_4_20_OR_LATER
	FMovieSceneFloatChannel RTPCChannel;
#endif

private:
#if WITH_EDITOR
	bool IsRTPCNameValid();

	FString PreviousName;
#endif
};
