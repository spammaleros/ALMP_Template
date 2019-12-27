// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
	AkAudioBankGenerationHelpers.h: Wwise Helpers to generate banks from the editor and when cooking.
------------------------------------------------------------------------------------*/
#pragma once

#include "AkAuxBus.h"
#include "AkAudioEvent.h"
#include "AkAudioBank.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAkBanks, Log, All);

namespace WwiseBnkGenHelper
{
	DECLARE_DELEGATE_OneParam(FOnBankGenerationComplete, bool);

	/**
	 * Get an absolute path to the associated Wwise project
	 */
	FString GetLinkedProjectPath();

	/**
	 * Dump the bank definition to file
	 *
	 * @param in_DefinitionFileContent	Banks to include in file
	 */
	bool GenerateDefinitionFile(TArray< TSharedPtr<FString> >& BanksToGenerate, TMap<FString, TSet<UAkAudioEvent*> >& BankToEventSet);
	
	/**
	 * Generate the Wwise soundbanks in a blocking process
	 *
	 * @param in_rBankNames				Names of the banks to generate
	 * @param in_PlatformNames			Names of the platforms to generate for
	 * @param WwisePathOverride			Use a different path to WwiseCli than the one in the ini file
	 */
	int32 GenerateSoundBanksBlocking(TArray< TSharedPtr<FString> >& in_rBankNames, TArray< TSharedPtr<FString> >& in_PlatformNames, const FString* WwisePathOverride = nullptr);

	/**
	 * Generate the Wwise soundbanks in a non-blocking manner
	 *
	 * @param in_rBankNames					Names of the banks to generate
	 * @param in_PlatformNames				Names of the platforms to generate for
	 * @param OnSoundBankGenerationComplete	Delegate fired when generation completes, take a bool argument indicating success
	 */
	void GenerateSoundBanksNonBlocking( TArray< TSharedPtr<FString> >& in_rBankNames, TArray< TSharedPtr<FString> >& in_PlatformNames, FOnBankGenerationComplete OnSoundBankGenerationComplete);

	/**
	 * Gather attenuation info for newly generated banks
	 *
	 * @params BankToEventSet	Map of SoundBanks to Events contained in that SoundBank
	 * 
	 */
	void FetchAttenuationInfo(const TMap<FString, TSet<UAkAudioEvent*> >& BankToEventSet);

	/**
	 * Function to create the Generate SoundBanks window
	 *
	 * @param pSoundBanks				List of SOundBanks to be pre-selected
	 * @paramin_bShouldSaveWwiseProject	Whether the Wwise project should be saved or not
	 *
	 */
	void CreateGenerateSoundBankWindow(TArray<TWeakObjectPtr<UAkAudioBank>>* pSoundBanks, bool in_bShouldSaveWwiseProject = false);
}
