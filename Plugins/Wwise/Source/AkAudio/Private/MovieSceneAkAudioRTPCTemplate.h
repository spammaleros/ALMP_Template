// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AkUEFeatures.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#if UE_4_20_OR_LATER
#include "Channels/MovieSceneFloatChannel.h"
#endif
#include "MovieSceneAkAudioRTPCTemplate.generated.h"


class UMovieSceneAkAudioRTPCSection;

struct FMovieSceneAkAudioRTPCSectionData
{
	FMovieSceneAkAudioRTPCSectionData() {}

	FMovieSceneAkAudioRTPCSectionData(const UMovieSceneAkAudioRTPCSection& Section);

	FString RTPCName;

#if UE_4_20_OR_LATER
	FMovieSceneFloatChannel RTPCChannel;
#else
	FRichCurve RTPCCurve;
#endif

};


USTRUCT()
struct AKAUDIO_API FMovieSceneAkAudioRTPCTemplate
	: public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

	FMovieSceneAkAudioRTPCTemplate() {}

	FMovieSceneAkAudioRTPCTemplate(const UMovieSceneAkAudioRTPCSection& InSection);

	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

	virtual void Setup(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;
	virtual void SetupOverrides() override { EnableOverrides(RequiresSetupFlag); }

	UPROPERTY()
	const UMovieSceneAkAudioRTPCSection* Section = nullptr;
};
