// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
AkWaapiUri.cpp
------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------
includes.
------------------------------------------------------------------------------------*/
#include "AkWaapiBlueprints/AkWaapiUri.h"
#include "AkAudioDevice.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SButton.h"
#include "Misc/ScopedSlowTask.h"
#include "Framework/Application/SlateApplication.h"
#include "AkAudioStyle.h"

/*------------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------------*/
#define LOCTEXT_NAMESPACE "AkAudio"

DEFINE_LOG_CATEGORY(LogAkUri);

/*------------------------------------------------------------------------------------
Statics and Globals
------------------------------------------------------------------------------------*/
namespace SAkWaapiUri_Helpers
{
	static const FString FullUriList[] =
	{
		TEXT("ak.wwise.core.object.get"),
		TEXT("ak.wwise.core.object.setName"),
		TEXT("ak.wwise.core.object.setProperty"),
		TEXT("ak.wwise.core.transport.executeAction"),
		TEXT("ak.wwise.ui.commands.execute"),
		TEXT("ak.wwise.core.object.setProperty"),
		TEXT("ak.wwise.core.transport.create"),
		TEXT("ak.wwise.core.transport.executeAction"),
		TEXT("ak.wwise.core.object.nameChanged"),
		TEXT("ak.wwise.core.object.propertyChanged")
		
	};

	enum { FullUriListSize = sizeof(FullUriList) / sizeof(*FullUriList) };
}
/*------------------------------------------------------------------------------------
Helpers
------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------
UAkWaapiUriConv
------------------------------------------------------------------------------------*/
UAkWaapiUriConv::UAkWaapiUriConv(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
}

FString UAkWaapiUriConv::Conv_FAkWaapiUriToString(const FAkWaapiUri& INAkWaapiUri)
{
	return INAkWaapiUri.Uri;
}

FText UAkWaapiUriConv::Conv_FAkWaapiUriToText(const FAkWaapiUri& INAkWaapiUri)
{
	return FText::FromString(*INAkWaapiUri.Uri);
}

/*------------------------------------------------------------------------------------
SAkWaapiUri
------------------------------------------------------------------------------------*/
SAkWaapiUri::SAkWaapiUri()
{}

SAkWaapiUri::~SAkWaapiUri()
{}

void SAkWaapiUri::Construct(const FArguments& InArgs)
{
	OnDragDetected = InArgs._OnDragDetected;
	OnSelectionChanged = InArgs._OnSelectionChanged;

	if (InArgs._FocusSearchBoxWhenOpened)
	{
		RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SAkWaapiUri::SetFocusPostConstruct));
	}

	SearchBoxFilter = MakeShareable(new StringFilter(StringFilter::FItemToStringArray::CreateSP(this, &SAkWaapiUri::PopulateSearchStrings)));
	SearchBoxFilter->OnChanged().AddSP(this, &SAkWaapiUri::FilterUpdated);

	ChildSlot
		[
			SNew(SBorder)
			.Padding(4)
			.BorderImage(FAkAudioStyle::GetBrush("AudiokineticTools.GroupBorder"))
			[
				SNew(SVerticalBox)
				// Search
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 1, 0, 3)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						InArgs._SearchContent.Widget
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(SearchBoxPtr, SSearchBox)
						.HintText(LOCTEXT("UriSearchHint", "Search a Uri"))
						.ToolTipText(LOCTEXT("UriSearchTooltip", "Type here to search for a Uri"))
						.OnTextChanged(SearchBoxFilter.Get(), &StringFilter::SetRawFilterText)
						.SelectAllTextWhenFocused(false)
						.DelayChangeNotificationsWhileTyping(true)
					]
				]
				// Tree
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(ListViewPtr, SListView< TSharedPtr<FString> >)
					.ListItemsSource(&UriList)
					.OnGenerateRow(this, &SAkWaapiUri::GenerateRow)
					.ItemHeight(18)
					.SelectionMode(InArgs._SelectionMode)
					.OnSelectionChanged(this, &SAkWaapiUri::ListSelectionChanged)
					.ClearSelectionOnClick(false)
				]
			]
		];

	for (const auto& Uri : SAkWaapiUri_Helpers::FullUriList)
	{
		UriList.Add(MakeShareable(new FString(Uri)));
	}
	UriList.Sort([](TSharedPtr< FString > Firststr, TSharedPtr< FString > Secondstr) { return *Firststr.Get() < *Secondstr.Get(); });
	ListViewPtr->RequestListRefresh();
}

TSharedRef<ITableRow> SAkWaapiUri::GenerateRow(TSharedPtr<FString> in_Uri, const TSharedRef<STableViewBase>& OwnerTable)
{
	check(in_Uri.IsValid());

	TSharedPtr<ITableRow> NewRow = SNew(STableRow< TSharedPtr<FString> >, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*in_Uri.Get()))
		.HighlightText(SearchBoxFilter.Get(), &StringFilter::GetRawFilterText)
		];
	return NewRow.ToSharedRef();
}

void SAkWaapiUri::PopulateSearchStrings(const FString& in_Uri, OUT TArray< FString >& OutSearchStrings) const
{
	OutSearchStrings.Add(in_Uri);
}

void SAkWaapiUri::FilterUpdated()
{
	FScopedSlowTask SlowTask(2.f, LOCTEXT("AK_PopulatingPicker", "Populating Uri Picker..."));
	SlowTask.MakeDialog();

	UriList.Empty(SAkWaapiUri_Helpers::FullUriListSize);

	FString FilterString = SearchBoxFilter->GetRawFilterText().ToString();
	if (FilterString.IsEmpty())
	{
		for (const auto& Uri : SAkWaapiUri_Helpers::FullUriList)
		{
			UriList.Add(MakeShareable(new FString(Uri)));
		}
	}
	else
	{
		for (const auto& Uri : SAkWaapiUri_Helpers::FullUriList)
		{
			if (Uri.Contains(FilterString))
			{
				UriList.Add(MakeShareable(new FString(Uri)));
			}
		}
	}
	UriList.Sort([](TSharedPtr< FString > Firststr, TSharedPtr< FString > Secondstr) { return *Firststr.Get() < *Secondstr.Get(); });
	ListViewPtr->RequestListRefresh();
}

void SAkWaapiUri::ListSelectionChanged(TSharedPtr< FString > in_Uri, ESelectInfo::Type /*SelectInfo*/)
{
	if (OnSelectionChanged.IsBound())
		OnSelectionChanged.Execute(in_Uri, ESelectInfo::OnMouseClick);
}

const TArray<TSharedPtr<FString>> SAkWaapiUri::GetSelectedUri() const
{
	return ListViewPtr->GetSelectedItems();
}

EActiveTimerReturnType SAkWaapiUri::SetFocusPostConstruct(double InCurrentTime, float InDeltaTime)
{
	FWidgetPath WidgetToFocusPath;
	FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchBoxPtr.ToSharedRef(), WidgetToFocusPath);
	FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);

	return EActiveTimerReturnType::Stop;
}
#undef LOCTEXT_NAMESPACE