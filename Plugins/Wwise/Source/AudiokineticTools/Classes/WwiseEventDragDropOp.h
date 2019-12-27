// Copyright (c) 2006-2012 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
	WwiseEventDragDropOp.h
------------------------------------------------------------------------------------*/
#pragma once

#include "WaapiPicker/WwiseTreeItem.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "ContentBrowserDelegates.h"

class FWwiseEventDragDropOp : public FDecoratedDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FWwiseEventDragDropOp, FDecoratedDragDropOp)

	const FSlateBrush* Icon;
	
	static TSharedRef<FWwiseEventDragDropOp> New(TArray<TSharedPtr<FWwiseTreeItem>> InAssets);
	~FWwiseEventDragDropOp();
	virtual FCursorReply OnCursorQuery() override;

	void SetCanDrop(const bool CanDrop);
	bool OnAssetViewDrop(const FAssetViewDragAndDropExtender::FPayload& Payload);
	bool OnAssetViewDragOver(const FAssetViewDragAndDropExtender::FPayload& Payload);
	bool OnAssetViewDragLeave(const FAssetViewDragAndDropExtender::FPayload& Payload);
	void RecurseCreateAssets(TSharedPtr<FWwiseTreeItem>& Asset, const FString& PackagePath);

	const TArray<TSharedPtr<FWwiseTreeItem>>& GetWiseItems() const;

	void SetTooltipText();
	FText GetTooltipText() const;
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
	FAssetViewDragAndDropExtender* pExtender;

private:	
	
	/** Data for the asset this item represents */
	TArray<TSharedPtr<FWwiseTreeItem>> WwiseAssets;

	bool CanDrop;
};