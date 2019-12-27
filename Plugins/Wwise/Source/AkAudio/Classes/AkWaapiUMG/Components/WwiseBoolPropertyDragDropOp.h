// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/DragAndDrop.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "AkAudioStyle.h"

class AKAUDIO_API FWwiseBoolPropertyDragDropOp : public FDragDropOperation
{
public:

	DRAG_DROP_OPERATOR_TYPE(FWwiseBoolPropertyDragDropOp, FDragDropOperation)

	static TSharedRef<FWwiseBoolPropertyDragDropOp> New(const TArray<TSharedPtr<FString>>& in_Wwiseproperties)
	{
		TSharedRef<FWwiseBoolPropertyDragDropOp> Operation = MakeShareable(new FWwiseBoolPropertyDragDropOp);
		
		Operation->MouseCursor = EMouseCursor::GrabHandClosed;
		Operation->Wwiseproperties = in_Wwiseproperties;
		Operation->Construct();

		return Operation;
	}
	
	const TArray< TSharedPtr<FString> >& GetWiseProperties() const
	{
		return Wwiseproperties;
	}

public:
	FText GetDecoratorText() const
	{
		FString Text = Wwiseproperties.Num() == 1 ? *Wwiseproperties[0].Get() : TEXT("");

		if (Wwiseproperties.Num() > 1 )
		{
#if UE_4_20_OR_LATER
			Text = FString::Printf(TEXT("Can't handle more than one Property"));
#else
			Text = FString::Printf(*NSLOCTEXT("AkSlider", "WwisePropperty", ": Can't handle more than one Property").ToString());
#endif
		}
		return FText::FromString(Text);
	}

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return 
			SNew(SBorder)
			.BorderImage(FAkAudioStyle::GetBrush("AudiokineticTools.AssetDragDropTooltipBackground"))
			.Content()
			[
				SNew(SHorizontalBox)
				// slot for the item name.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(3,0,3,0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock) 
						.Text(this, &FWwiseBoolPropertyDragDropOp::GetDecoratorText)
					]
				]
			];
	}

private:
	/** Data for the asset this item represents */
	TArray<TSharedPtr<FString>> Wwiseproperties;
};
