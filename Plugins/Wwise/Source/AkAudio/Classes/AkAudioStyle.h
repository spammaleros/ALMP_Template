// Copyright (c) 2017 Audiokinetic Inc. / All Rights Reserved
#pragma once

#include "WaapiPicker/WwiseTreeItem.h"

/**
* Implements the visual style of Wwise Picker.
*/
class AKAUDIO_API FAkAudioStyle
{
public:
	static const ISlateStyle& Get();
	static void Shutdown();

	static FName GetStyleSetName();

	static const FSlateBrush* GetBrush(EWwiseTreeItemType::Type ItemType);
	static const FSlateBrush* GetBrush(FName PropertyName, const ANSICHAR* Specifier = NULL);
	static const FSlateFontInfo GetFontStyle(FName PropertyName, const ANSICHAR* Specifier = NULL);

private:
	static TSharedPtr< class FSlateStyleSet > StyleInstance;
	static void Initialize();
};
