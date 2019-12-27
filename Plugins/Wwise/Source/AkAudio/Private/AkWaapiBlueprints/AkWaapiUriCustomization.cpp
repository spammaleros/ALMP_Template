// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#if WITH_EDITOR
#include "AkWaapiBlueprints/AkWaapiUriCustomization.h"
#include "AkAudioDevice.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "DetailWidgetRow.h"
#include "AkWaapiBlueprints/AkWaapiUri.h"
#include "AkAudioStyle.h"

#define LOCTEXT_NAMESPACE "AkWaapiUriCustomization"

TSharedRef<IPropertyTypeCustomization> FAkWaapiUriCustomization::MakeInstance()
{
	return MakeShareable(new FAkWaapiUriCustomization());
}

void FAkWaapiUriCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	UriHandle = StructPropertyHandle->GetChildHandle("Uri");

	if (UriHandle.IsValid())
	{
		TSharedPtr<SWidget> PickerWidget = nullptr;

		PickerWidget = SAssignNew(PickerButton, SButton)
			.ButtonStyle(FAkAudioStyle::Get(), "AudiokineticTools.HoverHintOnly")
			.ToolTipText(LOCTEXT("WwiseUriToolTipText", "Choose A Uri"))
			.OnClicked(FOnClicked::CreateSP(this, &FAkWaapiUriCustomization::OnPickContent, UriHandle.ToSharedRef()))
			.ContentPadding(2.0f)
			.ForegroundColor(FSlateColor::UseForeground())
			.IsFocusable(false)
			[
				SNew(SImage)
				.Image(FAkAudioStyle::GetBrush("AudiokineticTools.Button_EllipsisIcon"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			];

			HeaderRow.ValueContent()
			.MinDesiredWidth(125.0f)
			.MaxDesiredWidth(600.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					UriHandle->CreatePropertyValueWidget()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
				.VAlign(VAlign_Center)
				[
					PickerWidget.ToSharedRef()
				]
			]
			.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FAkWaapiUriCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

FReply FAkWaapiUriCustomization::OnPickContent(TSharedRef<IPropertyHandle> PropertyHandle)
{
	Window = SNew(SWindow)
		.Title(LOCTEXT("UriPickerWindowTitle", "Choose A Uri"))
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(FVector2D(350, 400));

	Window->SetContent(
		SNew(SBorder)
		[
			SNew(SAkWaapiUri)
			.FocusSearchBoxWhenOpened(true)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged(this, &FAkWaapiUriCustomization::UriSelectionChanged)
		]
	);

	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	FSlateApplication::Get().AddWindowAsNativeChild(Window.ToSharedRef(), RootWindow.ToSharedRef());
	return FReply::Handled();
}

void FAkWaapiUriCustomization::UriSelectionChanged(TSharedPtr< FString > in_Uri, ESelectInfo::Type SelectInfo)
{
	if (in_Uri.IsValid())
	{
		UriHandle->SetValue(*in_Uri.Get());
		Window->RequestDestroyWindow();
	}
}
#undef LOCTEXT_NAMESPACE

#endif//WITH_EDITOR