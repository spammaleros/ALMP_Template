// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateSoundBanksCommandlet.generated.h"

/**
 * 
 */
UCLASS()
class AUDIOKINETICTOOLS_API UGenerateSoundBanksCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:

	UGenerateSoundBanksCommandlet();

	// UCommandlet interface
	virtual int32 Main(const FString& Params) override;

private:
	void PrintHelp() const;
};
