// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*=============================================================================
	AkJsonFactory.cpp:
=============================================================================*/
#include "AkJsonFactory.h"
#include "AkAudioEvent.h"
#include "AkSettings.h"
#include "Misc/Paths.h"

/*------------------------------------------------------------------------------
	UAkJsonFactory.
------------------------------------------------------------------------------*/
UAkJsonFactory::UAkJsonFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAudioEvent::StaticClass();
	Formats.Add(TEXT("json;Audiokinetic SoundBank Metadata"));
	bCreateNew = true;
	bEditorImport = true;
	ImportPriority = 101;
}

UObject* UAkJsonFactory::FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	return nullptr;
}

bool UAkJsonFactory::FactoryCanImport(const FString& Filename)
{
	//check extension
	if (FPaths::GetExtension(Filename) == TEXT("json"))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();

		if (Filename.Contains(AkSettings ? AkSettings->WwiseSoundBankFolder.Path : UAkSettings::DefaultSoundBankFolder))
		{
			return true;
		}
	}

	return false;
}

bool UAkJsonFactory::ShouldShowInNewMenu() const
{
	return false;
}
