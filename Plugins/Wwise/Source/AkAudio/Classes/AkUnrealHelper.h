// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"

namespace AkUnrealHelper
{
	AKAUDIO_API void TrimPath(FString& Path);

	AKAUDIO_API FString GetProjectDirectory();

	AKAUDIO_API FString GetContentDirectory();

	AKAUDIO_API FString GetSoundBankDirectory();

#if WITH_EDITOR
	AKAUDIO_API void SanitizePath(FString& Path, const FString& PreviousPath, const FText& DialogMessage);
	AKAUDIO_API void SanitizeProjectPath(FString& Path, const FString& PreviousPath, const FText& DialogMessage, bool &bRequestRefresh);
#endif
}
