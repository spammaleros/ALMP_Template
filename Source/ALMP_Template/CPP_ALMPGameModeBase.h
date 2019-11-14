// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


//#include "Windows/MinWindows.h"
//#include <AK/SoundEngine/Common/AkSoundEngine.h>

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPP_ALMPGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class ALMP_TEMPLATE_API ACPP_ALMPGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
	/** Initialize the GameState actor. */
	virtual void InitGameState() override;


};
