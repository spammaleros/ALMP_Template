// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"


class SButton;
class SComboButton;
class IMenu;

class AKAUDIO_API FAkWaapiFieldNamesCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	/** Delegate used to display the Waapi Picker */
	FReply OnPickContent(TSharedRef<IPropertyHandle> PropertyHandle);

	/** Handler for tree view selection changes */
	void FieldNameSelectionChanged(TSharedPtr< FString > in_FieldName, ESelectInfo::Type SelectInfo);

private:

	/** The pick button widget */
	TSharedPtr<SButton> PickerButton;

	TSharedPtr<IPropertyHandle> FieldNameHandle;

	TSharedPtr<SWindow> Window;
};

#endif// WITH_EDITOR
