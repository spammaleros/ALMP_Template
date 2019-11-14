// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

/*------------------------------------------------------------------------------------
AkSlider.cpp
------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------
includes.
------------------------------------------------------------------------------------*/

#include "AkWaapiUMG/Components/AkWwiseTree.h"
#include "AkAudioDevice.h"
#include "Widgets/Input/SButton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "AkWaapiUMG/Components/WwiseUmgDragDropOp.h"
#include "AkWaapiSlate/Widgets/Input/AkSSlider.h"
/*------------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------------*/
#define LOCTEXT_NAMESPACE "AkWaapiUMG"

/*------------------------------------------------------------------------------------
Implementation
------------------------------------------------------------------------------------*/
/////////////////////////////////////////////////////
// UAkWwiseTree

UAkWwiseTree::UAkWwiseTree(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AkSSlider::FArguments Defaults;
}

TSharedRef<SWidget> UAkWwiseTree::RebuildWidget()
{
    bool autoFocus = GetWorld()->WorldType != EWorldType::Game &&
                     GetWorld()->WorldType != EWorldType::PIE &&
                     GetWorld()->WorldType != EWorldType::GamePreview;
	return
	SNew(SBorder)
	.BorderImage(FAkAudioStyle::GetBrush("AudiokineticTools.GroupBorder"))
	.Padding(FMargin(0.0f, 3.0f))
	.BorderBackgroundColor(FLinearColor(.6, .6, .6, 1.0f))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(ItemTextBlock, STextBlock)
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("editable_Tooltip", "Item Name"))
				.MinDesiredWidth(300.f)
				.Text(FText::FromString(TEXT("Choose an item from the Wwise tree")))
			]
		]
		+ SVerticalBox::Slot()
		[
			SAssignNew(WaapiPicker, SWaapiPicker)
			.RestrictContextMenu(true)
			.FocusSearchBoxWhenOpened(autoFocus)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged(FOnSelectionChanged::CreateUObject(this, &UAkWwiseTree::TreeSelectionChanged))
			.OnDragDetected(FOnDragDetected::CreateUObject(this, &UAkWwiseTree::HandleOnDragDetected))
		]
	];
}

void UAkWwiseTree::SynchronizeProperties()
{
	Super::SynchronizeProperties();

}

void UAkWwiseTree::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	WaapiPicker.Reset();
}

void UAkWwiseTree::TreeSelectionChanged(TSharedPtr< FWwiseTreeItem > TreeItem, ESelectInfo::Type SelectInfo)
{
	if (TreeItem.IsValid())
	{
		ItemTextBlock->SetText(FText::FromString(TreeItem->DisplayName));

		if (OnSelectionChanged.IsBound())
		{
			OnSelectionChanged.Broadcast(TreeItem->ItemId);
		}
	}
}

FReply UAkWwiseTree::HandleOnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && WaapiPicker.IsValid())
	{
		const TArray<TSharedPtr<FWwiseTreeItem>>& ItemsSelected = WaapiPicker->GetSelectedItems();
		if (ItemsSelected.Num() == 1)
		{		
			if(OnItemDragged.IsBound())
			{
				OnItemDragged.Broadcast(ItemsSelected[0]->ItemId, ItemsSelected[0]->DisplayName);
			}
			return FReply::Handled().BeginDragDrop(FWwiseUmgDragDropOp::New(ItemsSelected));
		}
	}
	return FReply::Unhandled();
}

FAkWwiseObjectDetails UAkWwiseTree::GetSelectedItem() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> selectedItems = WaapiPicker->GetSelectedItems();
	
	FAkWwiseObjectDetails itemObjectDetails = FAkWwiseObjectDetails();
	if (selectedItems.Num())
	{
		TSharedPtr<FWwiseTreeItem> itemSelected = selectedItems[0];
		itemObjectDetails.ItemId = itemSelected->ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces);
		itemObjectDetails.ItemName = itemSelected->DisplayName;
		itemObjectDetails.ItemPath = itemSelected->FolderPath;
	}
	return itemObjectDetails;
}

FString UAkWwiseTree::GetSearchText() const
{
	return WaapiPicker->GetSearchText();
}

void UAkWwiseTree::SetSearchText(const FString& newText)
{
	WaapiPicker->SetSearchText(newText);
}


#if WITH_EDITOR

const FText UAkWwiseTree::GetPaletteCategory()
{
	return LOCTEXT("Wwise", "Wwise");
}

#endif
//
/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
