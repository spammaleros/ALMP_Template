// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
	SWaapiPicker.cpp
------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------
 includes.
------------------------------------------------------------------------------------*/
#include "WaapiPicker/SWaapiPicker.h"
#include "AkAudioDevice.h"
#if WITH_EDITOR
#include "DirectoryWatcherModule.h"
#include "Modules/ModuleManager.h"
#endif//WITH_EDITOR
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "WaapiPicker/SWaapiPickerRow.h"
#include "WaapiPicker/WaapiPickerViewCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/ScopedSlowTask.h"
#include "AkWaapiUtils.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "WaapiPicker/SWaapiPickerRow.h"
#include "HAL/PlatformProcess.h"
#include "Framework/Application/SlateApplication.h"
#include "Async/Async.h"
#include "Modules/ModuleManager.h"

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 15 && WITH_EDITOR
#include "IDirectoryWatcher.h"
#endif
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SHyperlink.h"

/*------------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------------*/
#define LOCTEXT_NAMESPACE "AkAudio"

DEFINE_LOG_CATEGORY(LogAkAudioPicker);

/*------------------------------------------------------------------------------------
Statics and Globals
------------------------------------------------------------------------------------*/

const FName SWaapiPicker::WaapiPickerTabName = FName("WaapiPicker");

/*------------------------------------------------------------------------------------
Helpers
------------------------------------------------------------------------------------*/
static inline void InitiliseAkSettings(bool requestRefresh)
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	AkSettings->bRequestRefresh = requestRefresh;
}

static inline void CallWaapiGetProjectNamePath(FString& ProjectName, FString& ProjectPath)
{
	// Construct the arguments Json object : Getting infos.
	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	{
		TSharedPtr<FJsonObject> OfType = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> OfTypeJsonArray;
		OfTypeJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::PROJECT)));
		OfType->SetArrayField(WwiseWaapiHelper::OF_TYPE, OfTypeJsonArray);

		args->SetObjectField(WwiseWaapiHelper::FROM, OfType);
	}
	// Construct the options Json object : Getting specific infos to construct the wwise tree item "id - name - type - childrenCount - workunit:type - path - parent"
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	{
		TArray<TSharedPtr<FJsonValue>> StructJsonArray;
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::NAME)));
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::FILEPATH)));

		options->SetArrayField(WwiseWaapiHelper::RETURN, StructJsonArray);
	}
	
	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		TSharedPtr<FJsonObject> outJsonResult;
		// Request data from Wwise using WAAPI
		if (waapiClient->Call(ak::wwise::core::object::get, args, options, outJsonResult))
		{
			// Recover the information from the Json object getResult and use it to get the item id.
			TArray<TSharedPtr<FJsonValue>> StructJsonArray = outJsonResult->GetArrayField(WwiseWaapiHelper::RETURN);
			if (StructJsonArray.Num())
			{
				const TSharedPtr<FJsonObject>& ItemInfoObj = StructJsonArray[0]->AsObject();
				ProjectPath = FPaths::GetPath(ItemInfoObj->GetStringField(WwiseWaapiHelper::FILEPATH));
				ProjectName = FPaths::GetCleanFilename(ItemInfoObj->GetStringField(WwiseWaapiHelper::FILEPATH));
			}
			else
			{
				UE_LOG(LogAkAudioPicker, Log, TEXT("Unable to get the project name"));
			}
		}
	}
}

inline TSharedPtr<FWwiseTreeItem> SWaapiPicker::FindItemFromPath(const TSharedPtr<FWwiseTreeItem>& ParentItem,const FString& CurrentItemPath)
{
	// We get the element to create in an array and loop over it to create them.
	TArray<FString> itemPathArray;
	CurrentItemPath.ParseIntoArray(itemPathArray, *WwiseWaapiHelper::BACK_SLASH);
	TSharedPtr<FWwiseTreeItem> PreviousItem = ParentItem;
	for (int i = 1; i < itemPathArray.Num(); i++)
	{
		TSharedPtr<FWwiseTreeItem> ChildItem = PreviousItem->GetChild(itemPathArray[i]);
		if (!ChildItem.IsValid())
		{
			return TSharedPtr<FWwiseTreeItem>(NULL);
		}
		PreviousItem = ChildItem;
	}
	return  PreviousItem;
}

inline void SWaapiPicker::FindAndCreateItems(TSharedPtr<FWwiseTreeItem> CurrentItem)
{
	LastExpandedItems.Add(CurrentItem->ItemId);
	FString LastPathVisited = CurrentItem->FolderPath;
	LastPathVisited.RemoveFromEnd(WwiseWaapiHelper::BACK_SLASH + CurrentItem->DisplayName);
	TSharedPtr<FWwiseTreeItem> RootItem = GetRootItem(CurrentItem->FolderPath);
	if (CurrentItem->FolderPath == RootItem->FolderPath)
	{
		return;
	}
	else if (LastPathVisited == RootItem->FolderPath)
	{
		CurrentItem->Parent = RootItem->Parent.Pin();
		RootItem->Children.Add(CurrentItem);		
		return;
	}
	TSharedPtr<FWwiseTreeItem> ParentItem = FindItemFromPath(RootItem, LastPathVisited);
	if (ParentItem.IsValid())
	{
		CurrentItem->Parent = ParentItem->Parent.Pin();
		ParentItem->Children.Add(CurrentItem);		
	}
	else
	{
		TSharedPtr<FJsonObject> getResult;
		// Request data from Wwise UI using WAAPI and use them to create a Wwise tree item, getting the informations from a specific "PATH".
		if (CallWaapiGetInfoFrom(WwiseWaapiHelper::PATH, LastPathVisited, getResult, {}))
		{
			// Recover the information from the Json object getResult and use it to construct the tree item.
			TArray<TSharedPtr<FJsonValue>> StructJsonArray = getResult->GetArrayField(WwiseWaapiHelper::RETURN);
			TSharedPtr<FWwiseTreeItem> NewRootItem = ConstructWwiseTreeItem(StructJsonArray[0]);
			CurrentItem->Parent = NewRootItem;
			NewRootItem->Children.Add(CurrentItem);			
			FindAndCreateItems(NewRootItem);
		}
		else
		{
			UE_LOG(LogAkAudioPicker, Log, TEXT("Failed to get information from path : %s"), *LastPathVisited);
		}
	}
}

inline TSharedPtr<FWwiseTreeItem> SWaapiPicker::GetRootItem(const FString& InFullPath)
{
	for (int i = 0; i < EWwiseTreeItemType::NUM_DRAGGABLE_WAAPI_ITEMS; i++)
	{
		if (InFullPath.StartsWith(RootItems[i]->FolderPath))
		{
			return RootItems[i];
		}
	}
	return  TSharedPtr<FWwiseTreeItem>(NULL);
}

bool SWaapiPicker::CallWaapiGetInfoFrom(const FString& inFromField, const FString& inFromString, TSharedPtr<FJsonObject>& outJsonResult, const TArray<TransformStringField>& TransformFields)
{
	// Construct the arguments Json object : Getting infos "from - a specific id/path"
	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	{
		TSharedPtr<FJsonObject> from = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> fromJsonArray;
		fromJsonArray.Add(MakeShareable(new FJsonValueString(inFromString)));

		from->SetArrayField(inFromField, fromJsonArray);
		args->SetObjectField(WwiseWaapiHelper::FROM, from);

		// In case we would recover the children of the object that have the id : ID or the path : PATH, then we set isGetChildren to true.

		if (TransformFields.Num())
		{		
			TArray<TSharedPtr<FJsonValue>> transform;
			
			for (auto TransformValue : TransformFields)
			{
				TSharedPtr<FJsonObject> insideTransform = MakeShareable(new FJsonObject());
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				for (auto TransformStringValueArg : TransformValue.valueStringArgs)
				{
					JsonArray.Add(MakeShareable(new FJsonValueString(TransformStringValueArg)));
				}
				for (auto TransformNumberValueArg : TransformValue.valueNumberArgs)
				{
					JsonArray.Add(MakeShareable(new FJsonValueNumber(TransformNumberValueArg)));
				}
				insideTransform->SetArrayField(TransformValue.keyArg, JsonArray);
				transform.Add(MakeShareable(new FJsonValueObject(insideTransform)));
			}
			args->SetArrayField(WwiseWaapiHelper::TRANSFORM, transform);
		}
	}
	// Construct the options Json object : Getting specific infos to construct the wwise tree item "id - name - type - childrenCount - path - parent"
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	{
		TArray<TSharedPtr<FJsonValue>> StructJsonArray;
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::ID)));
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::NAME)));
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::TYPE)));
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::CHILDREN_COUNT)));
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::PATH)));

		options->SetArrayField(WwiseWaapiHelper::RETURN, StructJsonArray);
	}
	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		// Request data from Wwise using WAAPI
		if (waapiClient->Call(ak::wwise::core::object::get, args, options, outJsonResult))
		{
			return true;
		}
	}
	return false;
}

inline TSharedPtr<FWwiseTreeItem> SWaapiPicker::ConstructWwiseTreeItem(const TSharedPtr<FJsonValue>& inJsonItem)
{
	// Recover the data form the Json item, then use them to construct a Wwise tree item.
	const TSharedPtr<FJsonObject>& ItemInfoObj = inJsonItem->AsObject();
	const FString itemPath = ItemInfoObj->GetStringField(WwiseWaapiHelper::PATH);
	if (itemPath.StartsWith(WwiseWaapiHelper::BACK_SLASH + EWwiseTreeItemType::FolderNames[EWwiseTreeItemType::Event]) || itemPath.StartsWith(WwiseWaapiHelper::BACK_SLASH + EWwiseTreeItemType::FolderNames[EWwiseTreeItemType::AuxBus]) || itemPath.StartsWith(WwiseWaapiHelper::BACK_SLASH + EWwiseTreeItemType::FolderNames[EWwiseTreeItemType::ActorMixer]) || itemPath.StartsWith(WwiseWaapiHelper::BACK_SLASH + EWwiseTreeItemType::FolderNames[EWwiseTreeItemType::AcousticTexture]))
	{
		const FString itemIdString = ItemInfoObj->GetStringField(WwiseWaapiHelper::ID);
		FGuid in_ItemId = FGuid::NewGuid();
		FGuid::ParseExact(itemIdString, EGuidFormats::DigitsWithHyphensInBraces, in_ItemId);
		const FString itemName = ItemInfoObj->GetStringField(WwiseWaapiHelper::NAME);

		const uint32_t itemChilrenCount = ItemInfoObj->GetNumberField(WwiseWaapiHelper::CHILDREN_COUNT);
		const FString itemTypeString = ItemInfoObj->GetStringField(WwiseWaapiHelper::TYPE);

		EWwiseTreeItemType::Type itemType = EWwiseTreeItemType::FromString(itemTypeString);

		TSharedPtr<FWwiseTreeItem> treeItem = MakeShareable(new FWwiseTreeItem(itemName, itemPath, nullptr, itemType, in_ItemId));
		if ((itemType != EWwiseTreeItemType::Event) && (itemType != EWwiseTreeItemType::Sound))
		{
			treeItem->ChildCountInWwise = itemChilrenCount;
		}
		return treeItem;
	}
	else
	{
		return TSharedPtr<FWwiseTreeItem>(NULL);
	}
}

/*------------------------------------------------------------------------------------
Implementation
------------------------------------------------------------------------------------*/
SWaapiPicker::SWaapiPicker() : CommandList(MakeShareable(new FUICommandList))
{
	AllowTreeViewDelegates = true;
	isPickerVisible = FAkWaapiClient::IsProjectLoaded();
}

void SWaapiPicker::RemoveClientCallbacks()
{
	auto pWaapiClient = FAkWaapiClient::Get();
	if (pWaapiClient != nullptr)
	{
		if (ProjectLoadedHandle.IsValid())
		{
			pWaapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
			ProjectLoadedHandle.Reset();
		}
		if (ConnectionLostHandle.IsValid())
		{
			pWaapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
			ConnectionLostHandle.Reset();
		}
	}
}

void SWaapiPicker::OnProjectDirectoryChanged(const TArray<struct FFileChangeData>& ChangedFiles)
{
	// Code deleted : TO BE CONFIRMED.
}

SWaapiPicker::~SWaapiPicker()
{
	RootItems.Empty();

	RemoveClientCallbacks();
	//The following callback removal is intentionally left out of RemoveClientCallbacks()
	auto pWaapiClient = FAkWaapiClient::Get();
	if (pWaapiClient != nullptr)
	{
		pWaapiClient->OnClientBeginDestroy.Remove(ClientBeginDestroyHandle);
	}

	UnsubscribeAllWaapiCallbacks();
	StopAndDestroyAllTransports();

#if WITH_EDITOR
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	DirectoryWatcherModule.Get()->UnregisterDirectoryChangedCallback_Handle(ProjectFolder, ProjectDirectoryModifiedDelegateHandle); 
#endif//WITH_EDITOR
}

void SWaapiPicker::SubscribeAllWaapiCallbacks()
{
	if (FAkWaapiClient* waapiClient = FAkWaapiClient::Get())
	{
		if (waapiClient->IsConnected()
			&& (!idRenamed || !idPreDeleted || !idRemoved || !idChildAdded || !idCreated)
			)
		{
			auto wampEventCallback = WampEventCallback::CreateLambda([this](uint64_t id, TSharedPtr<FJsonObject> in_UEJsonObject)
			{
				// refresh the picker, since we got a notification of object created, deleted or name changed.
				AsyncTask(ENamedThreads::GameThread, [this]
				{
					ConstructTree();
				});
			});

			// Construct the options Json object : Getting parent infos.
			TSharedRef<FJsonObject> in_options = MakeShareable(new FJsonObject());
			{
				TArray<TSharedPtr<FJsonValue>> StructJsonArray;
				StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::PARENT)));
				in_options->SetArrayField(WwiseWaapiHelper::RETURN, StructJsonArray);
			}

			TSharedPtr<FJsonObject> unrealInfoJson;
			// Subscribe to object renamed-created-deleted and removed notifications.
			if (!idRenamed)
			{
				waapiClient->Subscribe(ak::wwise::core::object::nameChanged, in_options, wampEventCallback, idRenamed, unrealInfoJson);
			}
			if (!idPreDeleted)
			{
				waapiClient->Subscribe(ak::wwise::core::object::preDeleted, in_options, wampEventCallback, idPreDeleted, unrealInfoJson);
			}
			if (!idRemoved)
			{
				waapiClient->Subscribe(ak::wwise::core::object::childRemoved, in_options, wampEventCallback, idRemoved, unrealInfoJson);
			}
			if (!idChildAdded)
			{
				waapiClient->Subscribe(ak::wwise::core::object::childAdded, in_options, wampEventCallback, idChildAdded, unrealInfoJson);
			}
			if (!idCreated)
			{
				waapiClient->Subscribe(ak::wwise::core::object::created, in_options, wampEventCallback, idCreated, unrealInfoJson);
			}
		}
	}
}

void SWaapiPicker::UnsubscribeAllWaapiCallbacks()
{
	if (FAkWaapiClient* waapiClient = FAkWaapiClient::Get())
	{
		UnsubscribeWaapiCallback(waapiClient, idRenamed);
		UnsubscribeWaapiCallback(waapiClient, idPreDeleted);
		UnsubscribeWaapiCallback(waapiClient, idRemoved);
		UnsubscribeWaapiCallback(waapiClient, idChildAdded);
		UnsubscribeWaapiCallback(waapiClient, idCreated);
	}
}

void SWaapiPicker::UnsubscribeWaapiCallback(FAkWaapiClient* pClient, uint64& subscriptionID)
{
	if (subscriptionID > 0)
	{
		TSharedPtr<FJsonObject> result = MakeShareable(new FJsonObject());
		pClient->Unsubscribe(subscriptionID, result);
		subscriptionID = 0;
	}
}

void SWaapiPicker::RemoveAllWaapiCallbacks()
{
	if (FAkWaapiClient* waapiClient = FAkWaapiClient::Get())
	{
		RemoveWaapiCallback(waapiClient, idRenamed);
		RemoveWaapiCallback(waapiClient, idPreDeleted);
		RemoveWaapiCallback(waapiClient, idRemoved);
		RemoveWaapiCallback(waapiClient, idChildAdded);
		RemoveWaapiCallback(waapiClient, idCreated);
	}
}

void SWaapiPicker::RemoveWaapiCallback(FAkWaapiClient* pClient, uint64& subscriptionID)
{
	if (subscriptionID > 0)
	{
		pClient->RemoveWampEventCallback(subscriptionID);
		subscriptionID = 0;
	}
}

void SWaapiPicker::Construct(const FArguments& InArgs)
{
	OnDragDetected = InArgs._OnDragDetected;
	OnSelectionChanged = InArgs._OnSelectionChanged;

	CallWaapiGetProjectNamePath(ProjectName, ProjectFolder);
	bRestrictContextMenu = InArgs._RestrictContextMenu;

	if (InArgs._FocusSearchBoxWhenOpened)
	{
		RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SWaapiPicker::SetFocusPostConstruct));
	}
	FGenericCommands::Register();
	FWaapiPickerViewCommands::Register();
	CreateWaapiPickerCommands();

	SearchBoxFilter = MakeShareable( new StringFilter( StringFilter::FItemToStringArray::CreateSP( this, &SWaapiPicker::PopulateSearchStrings ) ) );
	SearchBoxFilter->OnChanged().AddSP( this, &SWaapiPicker::FilterUpdated );

	InitiliseAkSettings(false);
	
	ChildSlot
	[
		SNew(SBorder)
		.Padding(4)
		.BorderImage(FAkAudioStyle::GetBrush("AudiokineticTools.GroupBorder"))
		[
			SNew(SOverlay)

			// Picker
			+ SOverlay::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(this, &SWaapiPicker::isPickerAllowed)

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
						SAssignNew(SearchBoxPtr,SSearchBox)
						.HintText( LOCTEXT( "WaapiPickerSearchHint", "Search Wwise Item" ) )
						.ToolTipText(LOCTEXT("WaapiPickerSearchTooltip", "Type here to search for a Wwise asset"))
						.OnTextChanged( this, &SWaapiPicker::OnSearchBoxChanged )
						.SelectAllTextWhenFocused(false)
						.DelayChangeNotificationsWhileTyping(true)
					]
				]

				// Tree title
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(3.0f)
					[
						SNew(SImage) 
						.Image(FAkAudioStyle::GetBrush(EWwiseTreeItemType::Project))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0,0,3,0)
					[
						SNew(STextBlock)
						.Font(FAkAudioStyle::GetFontStyle("AudiokineticTools.SourceTitleFont") )
						.Text( this, &SWaapiPicker::GetProjectName )
						.Visibility(InArgs._ShowTreeTitle ? EVisibility::Visible : EVisibility::Collapsed)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1)
					[
						SNew( SSpacer )
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("AkPickerPopulate", "Populate"))
						.OnClicked(this, &SWaapiPicker::OnPopulateClicked)
					]
				]

				// Separator
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 1)
				[
					SNew(SSeparator)
					.Visibility( ( InArgs._ShowSeparator) ? EVisibility::Visible : EVisibility::Collapsed )
				]
				
				// Tree
				+SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(TreeViewPtr, STreeView< TSharedPtr<FWwiseTreeItem> >)
					.TreeItemsSource(&RootItems)
					.OnGenerateRow( this, &SWaapiPicker::GenerateRow )
					//.OnItemScrolledIntoView( this, &SPathView::TreeItemScrolledIntoView )
					.ItemHeight(18)
					.SelectionMode(InArgs._SelectionMode)
					.OnSelectionChanged(this, &SWaapiPicker::TreeSelectionChanged)
					.OnExpansionChanged(this, &SWaapiPicker::TreeExpansionChanged)
					.OnGetChildren( this, &SWaapiPicker::GetChildrenForTree )
					//.OnSetExpansionRecursive( this, &SPathView::SetTreeItemExpansionRecursive )
					.OnContextMenuOpening(this, &SWaapiPicker::MakeWaapiPickerContextMenu)
					.ClearSelectionOnClick(false)
				]
			]

			// Empty Picker
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Visibility(this, &SWaapiPicker::isWarningVisible)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("EmptyWaapiTree", "Could not establish a WAAPI connection; WAAPI picker is disabled. Please enable WAAPI in your Wwise settings, or use the Wwise Picker."))
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SHyperlink)
					.Visibility(this, &SWaapiPicker::isWarningVisible)
					.Text(LOCTEXT("WaapiDucumentation", "For more informaton, please Visit Waapi Documentation."))
					.ToolTipText(LOCTEXT("WaapiDucumentationTooltip", "Opens Waapi documentation in a new browser window"))
					.OnNavigate_Lambda([=]() { FPlatformProcess::LaunchURL(*FString("https://www.audiokinetic.com/library/?source=SDK&id=waapi.html"), nullptr, nullptr); })
				]
			]
		]
	];
	OnPopulateClicked();
	ExpandFirstLevel();

	// Subscribe to object renamed-created-deleted and removed to be notified from Wwise using WAAPI, so we can maintain the Wise picker up to date dynamically.
	SubscribeAllWaapiCallbacks();

	auto pWaapiClient = FAkWaapiClient::Get();
	if (pWaapiClient)
	{
		/* Construct the tree when we have the same project */
		ProjectLoadedHandle = pWaapiClient->OnProjectLoaded.AddLambda([this]()
		{
			SubscribeAllWaapiCallbacks();

			isPickerVisible = true;
			CallWaapiGetProjectNamePath(ProjectName, ProjectFolder);
			ConstructTree();
		});
		/* Empty the tree when we have different projects */
		ConnectionLostHandle = pWaapiClient->OnConnectionLost.AddLambda([this]()
		{
			isPickerVisible = false;

			RemoveAllWaapiCallbacks();
			ConstructTree();
		});
		ClientBeginDestroyHandle = pWaapiClient->OnClientBeginDestroy.AddLambda([this]()
		{
			RemoveClientCallbacks();
		});
	}
}

EVisibility SWaapiPicker::isPickerAllowed() const
{
	if (isPickerVisible)
		return EVisibility::Visible;
	return EVisibility::Hidden;
}

EVisibility SWaapiPicker::isWarningVisible() const
{
	if (!isPickerVisible)
		return EVisibility::Visible;
	return EVisibility::Hidden;
}

void SWaapiPicker::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if(AkSettings->bRequestRefresh)
	{
		ConstructTree();
		AkSettings->bRequestRefresh = false;
	}
}

FText SWaapiPicker::GetProjectName() const
{
	return FText::FromString(ProjectName);
}

FReply SWaapiPicker::OnPopulateClicked()
{
	ConstructTree();
	return FReply::Handled();
}

void SWaapiPicker::ConstructTree()
{
	if (FAkWaapiClient::IsProjectLoaded())
	{

		FString CurrentFilterText = SearchBoxFilter.IsValid() ? SearchBoxFilter->GetRawFilterText().ToString() : TEXT("");
		if (!CurrentFilterText.IsEmpty())
		{
			FilterUpdated();
			return;
		}
		RootItems.Empty(EWwiseTreeItemType::NUM_DRAGGABLE_WAAPI_ITEMS);
		EWwiseTreeItemType::Type CurrentType = EWwiseTreeItemType::Event;
		// Construct the three main roots ("Event", "AuxBus", "AcousticTexture", "ActorMixer") of the tree.
		while ((int) CurrentType < (int) EWwiseTreeItemType::NUM_DRAGGABLE_WAAPI_ITEMS)
		{
			FGuid in_ItemId = FGuid::NewGuid();
			TSharedPtr<FJsonObject> getResult;
			uint32_t itemChilrenCount = 0;
			FString path = WwiseWaapiHelper::BACK_SLASH + EWwiseTreeItemType::FolderNames[CurrentType];
			// Request data from Wwise UI using WAAPI and use them to create a Wwise tree item, getting the informations from a specific "PATH".
			if (CallWaapiGetInfoFrom(WwiseWaapiHelper::PATH, path, getResult, {}))
			{
				// Recover the information from the Json object getResult and use it to get the item id.
				TArray<TSharedPtr<FJsonValue>> StructJsonArray = getResult->GetArrayField(WwiseWaapiHelper::RETURN);
				const TSharedPtr<FJsonObject>& ItemInfoObj = StructJsonArray[0]->AsObject();
				const FString itemIdString = ItemInfoObj->GetStringField(WwiseWaapiHelper::ID);
				path = ItemInfoObj->GetStringField(WwiseWaapiHelper::PATH);
				itemChilrenCount = ItemInfoObj->GetNumberField(WwiseWaapiHelper::CHILDREN_COUNT);
				FGuid::ParseExact(itemIdString, EGuidFormats::DigitsWithHyphensInBraces, in_ItemId);
			}
			else
			{
				UE_LOG(LogAkAudioPicker, Log, TEXT("Failed to get information from id : %s"), *path);
			}
			// Create a new tree item and add it the root list.
			TSharedPtr<FWwiseTreeItem> NewRootParent = MakeShareable(new FWwiseTreeItem(EWwiseTreeItemType::ItemNames[CurrentType], path, nullptr, EWwiseTreeItemType::PhysicalFolder, in_ItemId));
			NewRootParent->ChildCountInWwise = itemChilrenCount;
			RootItems.Add(NewRootParent);
			CurrentType = (EWwiseTreeItemType::Type)(((int)CurrentType) + 1);
		}
		AllowTreeViewDelegates = true;
		ExpandFirstLevel();
	}
	RestoreTreeExpansion(RootItems);
	TreeViewPtr->RequestTreeRefresh();
}

void SWaapiPicker::ExpandFirstLevel()
{
	// Expand root items and first-level work units.
	for(int32 i = 0; i < RootItems.Num(); i++)
	{
		TreeViewPtr->SetItemExpansion(RootItems[i], true);
	}
}

void SWaapiPicker::ExpandParents(TSharedPtr<FWwiseTreeItem> Item)
{
	if(Item->Parent.IsValid())
	{
		ExpandParents(Item->Parent.Pin());
		TreeViewPtr->SetItemExpansion(Item->Parent.Pin(), true);
	}
}

TSharedRef<ITableRow> SWaapiPicker::GenerateRow( TSharedPtr<FWwiseTreeItem> TreeItem, const TSharedRef<STableViewBase>& OwnerTable )
{
	check(TreeItem.IsValid());
	
	EVisibility RowVisibility = TreeItem->IsVisible ? EVisibility::Visible : EVisibility::Collapsed;
	
	TSharedPtr<ITableRow> NewRow = SNew(STableRow< TSharedPtr<FWwiseTreeItem> >, OwnerTable)
		.OnDragDetected(this, &SWaapiPicker::HandleOnDragDetected)
		.Visibility(RowVisibility)
		[
			SNew(SWaapiPickerRow)
			.WaapiPickerItem(TreeItem)
			.HighlightText(this, &SWaapiPicker::GetHighlightText)
			.IsSelected(this, &SWaapiPicker::IsTreeItemSelected, TreeItem)
		];
	
	TreeItem->TreeRow = NewRow.Get();
	
	return NewRow.ToSharedRef();
}

void SWaapiPicker::GetChildrenForTree( TSharedPtr< FWwiseTreeItem > TreeItem, TArray< TSharedPtr<FWwiseTreeItem> >& OutChildren )
{	
	// In case the item is "unexpanded" and have children (in the Wwise tree), we need to add a default item to show the arrow that says, this item have children.
	FString CurrentFilterText = SearchBoxFilter->GetRawFilterText().ToString();
	if (TreeItem->ChildCountInWwise && CurrentFilterText.IsEmpty())
	{
		if (!LastExpandedItems.Contains(TreeItem->ItemId))
		{
			TreeItem->Children.Empty();
			TSharedPtr<FWwiseTreeItem> emptyTreeItem = MakeShareable(new FWwiseTreeItem(WwiseWaapiHelper::NAME, WwiseWaapiHelper::PATH, nullptr, EWwiseTreeItemType::PhysicalFolder, FGuid::NewGuid()));
			TreeItem->Children.Add(emptyTreeItem);
		}
		else
		{
			// Update the item expansion to be visible in the tree, since it is being expanded by the user.
			TreeViewPtr->SetItemExpansion(TreeItem, true);
		}
	}
	else if (!TreeItem->ChildCountInWwise)
	{
		// This is useful in case when the item contains elements that are being moved to an other path and the item
		// has no longer children and was expanded, so we need to remove it form the expansion items list.
		LastExpandedItems.Remove(TreeItem->ItemId);
	}
	OutChildren = TreeItem->Children;
}

FReply SWaapiPicker::HandleOnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{		
	// Refresh the contents
	if(OnDragDetected.IsBound())
		return OnDragDetected.Execute(Geometry,MouseEvent);

	return FReply::Unhandled();
}

void SWaapiPicker::PopulateSearchStrings( const FString& FolderName, OUT TArray< FString >& OutSearchStrings ) const
{
	OutSearchStrings.Add( FolderName );
}

void SWaapiPicker::OnSearchBoxChanged( const FText& InSearchText )
{
	SearchBoxFilter->SetRawFilterText( InSearchText );
}

FText SWaapiPicker::GetHighlightText() const
{
	return SearchBoxFilter->GetRawFilterText();
}

void SWaapiPicker::FilterUpdated()
{

	FScopedSlowTask SlowTask(2.f, LOCTEXT("AK_PopulatingPicker", "Populating Waapi Picker..."));
	SlowTask.MakeDialog();
	if (RootItems.Num())
	{
		ApplyFilter();
	}
	TreeViewPtr->RequestTreeRefresh();
}

void SWaapiPicker::SetItemVisibility(TSharedPtr<FWwiseTreeItem> Item, bool IsVisible)
{
	if( Item.IsValid() )
	{
		if( IsVisible )
		{
			// Propagate visibility to parents.
			SetItemVisibility(Item->Parent.Pin(), IsVisible);
		}
		Item->IsVisible = IsVisible;
		if( Item->TreeRow != NULL )
		{
			TSharedRef<SWidget> wid = Item->TreeRow->AsWidget();
			Item->TreeRow->AsWidget()->SetVisibility(IsVisible ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}

void SWaapiPicker::ApplyFilter()
{
	EWwiseTreeItemType::Type CurrentType = EWwiseTreeItemType::Event;
	// Construct the three main roots ("Event", "AuxBus", "AcousticTexture", "ActorMixer") of the tree.
	while ((int) CurrentType < (int) EWwiseTreeItemType::NUM_DRAGGABLE_WAAPI_ITEMS)
	{
		RootItems[CurrentType]->Children.Empty();
		CurrentType = (EWwiseTreeItemType::Type)(((int)CurrentType) + 1);
	}

	static 	TSet< FGuid > LastExpandedItemsBeforFilter;
	AllowTreeViewDelegates = false;
	FString CurrentFilterText = SearchBoxFilter->GetRawFilterText().ToString();
	if (!CurrentFilterText.IsEmpty())
	{
		if (!LastExpandedItemsBeforFilter.Num())
		{
			// We preserve the last expanded items to re-expand the tree as it was in non filtering mode.
			LastExpandedItemsBeforFilter = LastExpandedItems;
			LastExpandedItems.Empty();
		}

			TSharedPtr<FJsonObject> getResult;
			if (CallWaapiGetInfoFrom(WwiseWaapiHelper::SEARCH, CurrentFilterText, getResult, { { WwiseWaapiHelper::WHERE, { WwiseWaapiHelper::NAMECONTAINS, CurrentFilterText }, {} }, { WwiseWaapiHelper::RANGE, {}, { 0, 2000*CurrentFilterText.Len() } } }))
			{
				// Recover the information from the Json object getResult and use it to construct the tree item.
				TArray<TSharedPtr<FJsonValue>> SearchResultArray = getResult->GetArrayField(WwiseWaapiHelper::RETURN);
				if (SearchResultArray.Num())
				{
					// The map contains each path and the correspondent object of the search result.
					TMap < FString, TSharedPtr<FWwiseTreeItem>> SearchedResultTreeItem;
					for (int i = 0; i < SearchResultArray.Num(); i++)
					{
						// Fill the map with the path-object elements.
						TSharedPtr<FWwiseTreeItem> NewRootChild = ConstructWwiseTreeItem(SearchResultArray[i]);
						if (NewRootChild.IsValid())
						{
							FindAndCreateItems(NewRootChild);
						}
					}
				}
			}
			else
			{
				UE_LOG(LogAkAudioPicker, Log, TEXT("Failed to get information from item search : %s"), *CurrentFilterText);
			}
	}
	else
	{
		// Recover the last expanded items before filtering.
		LastExpandedItems.Empty();
		LastExpandedItems = LastExpandedItemsBeforFilter;
		LastExpandedItemsBeforFilter.Empty();
		AllowTreeViewDelegates = true;
		OnPopulateClicked();
		return;
	}
	RestoreTreeExpansion(RootItems);
	AllowTreeViewDelegates = true;
}

void SWaapiPicker::RestoreTreeExpansion(const TArray< TSharedPtr<FWwiseTreeItem> >& Items)
{
	for(int i = 0; i < Items.Num(); i++)
	{
		if(LastExpandedItems.Contains(Items[i]->ItemId) )
		{
			TreeViewPtr->SetItemExpansion(Items[i], true);
		}
		RestoreTreeExpansion(Items[i]->Children);
	}
}

void SWaapiPicker::TreeSelectionChanged( TSharedPtr< FWwiseTreeItem > TreeItem, ESelectInfo::Type /*SelectInfo*/ )
{
	if( AllowTreeViewDelegates )
	{
		const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();

		LastSelectedItems.Empty();
		for (int32 ItemIdx = 0; ItemIdx < SelectedItems.Num(); ++ItemIdx)
		{
			const TSharedPtr<FWwiseTreeItem> Item = SelectedItems[ItemIdx];
			if ( Item.IsValid() )
			{
				LastSelectedItems.Add(Item->ItemId);
			}
		}

		if (OnSelectionChanged.IsBound())
			OnSelectionChanged.Execute(TreeItem, ESelectInfo::OnMouseClick);
	}
}

void SWaapiPicker::TreeExpansionChanged( TSharedPtr< FWwiseTreeItem > TreeItem, bool bIsExpanded )
{
	if( AllowTreeViewDelegates )
	{
		// If the item is not expanded we don't need to request the server to get any information(the children are hidden).
		if (bIsExpanded)
		{
			LastExpandedItems.Add(TreeItem->ItemId);

			FString CurrentFilterText = SearchBoxFilter->GetRawFilterText().ToString();
			if (CurrentFilterText.IsEmpty())
			{
				FGuid in_ItemId = TreeItem->ItemId;
				const FString itemIdStringField = in_ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces);
				TSharedPtr<FJsonObject> getResult;

				// Request data from Wwise UI using WAAPI and use them to create a Wwise tree item, getting the informations from a specific "ID".
				if (CallWaapiGetInfoFrom(WwiseWaapiHelper::ID, itemIdStringField, getResult, { { WwiseWaapiHelper::SELECT, {WwiseWaapiHelper::CHILDREN}, {} } }))
				{
					// Recover the information from the Json object getResult and use it to construct the tree item.
					TArray<TSharedPtr<FJsonValue>> StructJsonArray = getResult->GetArrayField(WwiseWaapiHelper::RETURN);
					/** If the item have just one child and we are expanding it, this means that we need to construct the children list.
					*   In case the the number of children gotten form Wwise is not the same of the item, this means that there is some children added/removed,
					*   so we also need to construct the new list.
					*/
					if ((TreeItem->Children.Num() == 1) || (TreeItem->Children.Num() != StructJsonArray.Num()))
					{
						TreeItem->Children.Empty();
						for (int i = 0; i < StructJsonArray.Num(); i++)
						{
							TSharedPtr<FWwiseTreeItem> NewRootChild = ConstructWwiseTreeItem(StructJsonArray[i]);
							TreeItem->Children.Add(NewRootChild);
							NewRootChild->Parent = TreeItem;
						}
						TreeItem->SortChildren();
					}
				}
				else
				{
					UE_LOG(LogAkAudioPicker, Log, TEXT("Failed to get information from id : %s"), *itemIdStringField);
				}
			}
		}
		else
		{
			LastExpandedItems.Remove(TreeItem->ItemId);
		}	
	}
	else if(bIsExpanded)
	{
		TreeItem->SortChildren();
	}
}

bool SWaapiPicker::IsTreeItemSelected(TSharedPtr<FWwiseTreeItem> TreeItem) const
{
	return TreeViewPtr->IsItemSelected(TreeItem);
}

TSharedPtr<SWidget> SWaapiPicker::MakeWaapiPickerContextMenu()
{
	const FWaapiPickerViewCommands& Commands = FWaapiPickerViewCommands::Get();

	// Build up the menu
	FMenuBuilder MenuBuilder(true, CommandList);
	{
		MenuBuilder.BeginSection("WaapiPickerCreate", LOCTEXT("MenuHeader", "WaapiPicker"));
		{
			MenuBuilder.AddMenuEntry(Commands.RequestPlayWwiseItem);
			MenuBuilder.AddMenuEntry(Commands.RequestStopAllWwiseItem);
		}
		MenuBuilder.EndSection();
		MenuBuilder.BeginSection("WaapiPickerEdit", LOCTEXT("EditMenuHeader", "Edit"));
		{
			MenuBuilder.AddMenuEntry(Commands.RequestRenameWwiseItem);
			MenuBuilder.AddMenuEntry(Commands.RequestDeleteWwiseItem);
		}
		MenuBuilder.EndSection();
		if (!bRestrictContextMenu)
		{
			MenuBuilder.BeginSection("WaapiPickerExplore", LOCTEXT("ExploreMenuHeader", "Explore"));
			{
				MenuBuilder.AddMenuEntry(Commands.RequestExploreWwiseItem);
				MenuBuilder.AddMenuEntry(Commands.RequestFindInProjectExplorerWwisetem);
			}
			MenuBuilder.EndSection();
		}
		MenuBuilder.BeginSection("WaapiPickerRefreshAll");
		{
			MenuBuilder.AddMenuEntry(Commands.RequestRefreshWaapiPicker);

		}
		MenuBuilder.EndSection();
	}
	return MenuBuilder.MakeWidget();
}

void SWaapiPicker::CreateWaapiPickerCommands()
{
	const FWaapiPickerViewCommands& Commands = FWaapiPickerViewCommands::Get();
	FUICommandList& ActionList = *CommandList;

	// Action for rename a Wwise item.
	ActionList.MapAction(Commands.RequestRenameWwiseItem,
		FExecuteAction::CreateRaw(this, &SWaapiPicker::HandleRenameWwiseItemCommandExecute),
		FCanExecuteAction::CreateRaw(this, &SWaapiPicker::HandleRenameWwiseItemCommandCanExecute));

	// Action to play a Wwise item (event).
	ActionList.MapAction(Commands.RequestPlayWwiseItem,
		FExecuteAction::CreateRaw(this, &SWaapiPicker::HandlePlayWwiseItemCommandExecute),
		FCanExecuteAction::CreateRaw(this, &SWaapiPicker::HandlePlayWwiseItemCommandCanExecute));

	// Action to stop all playing Wwise item (event).
	ActionList.MapAction(Commands.RequestStopAllWwiseItem,
		FExecuteAction::CreateSP(this, &SWaapiPicker::StopAndDestroyAllTransports));

	// Action for rename a Wwise item.
	ActionList.MapAction(Commands.RequestDeleteWwiseItem,
		FExecuteAction::CreateRaw(this, &SWaapiPicker::HandleDeleteWwiseItemCommandExecute),
		FCanExecuteAction::CreateRaw(this, &SWaapiPicker::HandleDeleteWwiseItemCommandCanExecute));

	// Explore an item in the containing folder.
	ActionList.MapAction(Commands.RequestExploreWwiseItem,
		FExecuteAction::CreateRaw(this, &SWaapiPicker::HandleExploreWwiseItemCommandExecute),
		FCanExecuteAction::CreateRaw(this, &SWaapiPicker::HandleWwiseCommandCanExecute));

	// Explore an item in the containing folder.
	ActionList.MapAction(Commands.RequestFindInProjectExplorerWwisetem,
		FExecuteAction::CreateRaw(this, &SWaapiPicker::HandleFindWwiseItemInProjectExplorerCommandExecute),
		FCanExecuteAction::CreateRaw(this, &SWaapiPicker::HandleWwiseCommandCanExecute));

	// Action for refresh the Waapi Picker.
	ActionList.MapAction(Commands.RequestRefreshWaapiPicker,
		FExecuteAction::CreateSP(this, &SWaapiPicker::HandleRefreshWaapiPickerCommandExecute));

	// Action for undo last action in the Waapi Picker.
	ActionList.MapAction(
		FGenericCommands::Get().Undo,
		FExecuteAction::CreateSP(this, &SWaapiPicker::HandleUndoWaapiPickerCommandExecute));

	// Action for redo last action in the Waapi Picker.
	ActionList.MapAction(
		FGenericCommands::Get().Redo,
		FExecuteAction::CreateSP(this, &SWaapiPicker::HandleRedoWaapiPickerCommandExecute));
}


bool SWaapiPicker::HandleRenameWwiseItemCommandCanExecute() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	return (SelectedItems.Num() == 1 && (SelectedItems[0]->ItemType != EWwiseTreeItemType::PhysicalFolder) && (SelectedItems[0]->ItemType != EWwiseTreeItemType::StandaloneWorkUnit) && (SelectedItems[0]->ItemType != EWwiseTreeItemType::NestedWorkUnit));
}

void SWaapiPicker::HandleRenameWwiseItemCommandExecute() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	if (SelectedItems.Num())
	{
		TSharedPtr<ITableRow> TableRow = TreeViewPtr->WidgetFromItem(SelectedItems[0]);
		// If the Wwise item is selected but not visible, we scroll it into the view.
		if (!TableRow.IsValid())
		{
			TreeViewPtr->RequestScrollIntoView(SelectedItems[0]);
			return;
		}
		// Get the right Row to enter in editing mode.
		TSharedPtr<STableRow< TSharedPtr<FWwiseTreeItem>> > TableRowItem = StaticCastSharedPtr<STableRow< TSharedPtr<FWwiseTreeItem>>>(TableRow);
		if (TableRowItem.IsValid())
		{
			TSharedPtr<SWidget> RowContent = TableRowItem->GetContent();
			TSharedPtr<SWaapiPickerRow> ItemWidget = StaticCastSharedPtr<SWaapiPickerRow>(RowContent);
			if (ItemWidget.IsValid())
			{
				ItemWidget->EnterEditingMode();
			}
		}
	}
}

bool SWaapiPicker::HandlePlayWwiseItemCommandCanExecute() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	if (SelectedItems.Num() > 0)
	{
		for (int32 i = 0; i < SelectedItems.Num(); ++i)
		{
			if ((SelectedItems[i]->ItemType != EWwiseTreeItemType::Event) && (SelectedItems[i]->ItemType != EWwiseTreeItemType::Sound) && (SelectedItems[i]->ItemType != EWwiseTreeItemType::BlendContainer) &&
			(SelectedItems[i]->ItemType != EWwiseTreeItemType::SwitchContainer) && (SelectedItems[i]->ItemType != EWwiseTreeItemType::RandomSequenceContainer))
			{
				return false;
			}	
		}
		return true;
	}
	return false;
}

int32 SWaapiPicker::CreateTransport(const FGuid& in_ItemId)
{
	const FString itemIdStringField = in_ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces);
	TSharedPtr<FJsonObject> getResult;
	int32 transportID = -1;
	if (SWaapiPickerRow::CallWaapiExecuteUri(ak::wwise::core::transport::create, { { WwiseWaapiHelper::OBJECT, itemIdStringField } }, getResult))
	{
		transportID = getResult->GetIntegerField(WwiseWaapiHelper::TRANSPORT);
		uint64 subscriptionID = SubscribeToTransportStateChanged(transportID);
		ItemToTransport.Add(in_ItemId, TransportInfo(transportID, subscriptionID));
	}

	return transportID;
}

void SWaapiPicker::DestroyTransport(const FGuid& in_itemID)
{
	if(!ItemToTransport.Contains(in_itemID))
		return;

	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	args->SetNumberField(WwiseWaapiHelper::TRANSPORT, ItemToTransport[in_itemID].TransportID);

	// Construct the options Json object;
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> getResult;

	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		if(ItemToTransport[in_itemID].SubscriptionID != 0)
			waapiClient->Unsubscribe(ItemToTransport[in_itemID].SubscriptionID, getResult);

		if (waapiClient->Call(ak::wwise::core::transport::destroy, args, options, getResult))
			ItemToTransport.Remove(in_itemID);
	}
}

void SWaapiPicker::TogglePlayStop(int32 in_transportID)
{
	// Construct the arguments Json object : executing an action (play/stop).
	TSharedPtr<FJsonObject> getResult;
	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	args->SetStringField(WwiseWaapiHelper::ACTION, WwiseWaapiHelper::PLAYSTOP);
	args->SetNumberField(WwiseWaapiHelper::TRANSPORT, in_transportID);

	// Construct the options Json object;
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());

	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		// Request changes in Wwise using WAAPI
		if (!waapiClient->Call(ak::wwise::core::transport::executeAction, args, options, getResult))
		{
			UE_LOG(LogAkAudioPicker, Log, TEXT("Failed to trigger playback"));
		}

	}
	else
	{
		UE_LOG(LogAkAudioPicker, Log, TEXT("Unable to connect to localhost"));
	}
}

void SWaapiPicker::StopTransport(int32 in_transportID)
{
	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	args->SetStringField(WwiseWaapiHelper::ACTION, WwiseWaapiHelper::STOP);
	args->SetNumberField(WwiseWaapiHelper::TRANSPORT, in_transportID);

	// Construct the options Json object;
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> getResult;

	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		// Request changes in Wwise using WAAPI
		if (!waapiClient->Call(ak::wwise::core::transport::executeAction, args, options, getResult))
		{
			UE_LOG(LogAkAudioPicker, Log, TEXT("Cannot stop event."));
		}
	}
}

void SWaapiPicker::HandleStateChanged(TSharedPtr<FJsonObject> in_UEJsonObject)
{
	const FString newState = in_UEJsonObject->GetStringField(WwiseWaapiHelper::STATE);
	FGuid itemID;
	FGuid::Parse(in_UEJsonObject->GetStringField(WwiseWaapiHelper::OBJECT), itemID);
	const int32 transportID = in_UEJsonObject->GetNumberField(WwiseWaapiHelper::TRANSPORT);
	if (newState == WwiseWaapiHelper::STOPPED)
	{
		DestroyTransport(itemID);
	}
	else if (newState == WwiseWaapiHelper::PLAYING)
	{
		if (!ItemToTransport.Contains(itemID))
		{
			ItemToTransport.Add(itemID, TransportInfo(transportID, 0));
		}
	}
}

uint64 SWaapiPicker::SubscribeToTransportStateChanged(int32 in_transportID)
{
	auto wampEventCallback = WampEventCallback::CreateLambda([&](uint64_t id, TSharedPtr<FJsonObject> in_UEJsonObject)
	{
		AsyncTask(ENamedThreads::GameThread, [this, in_UEJsonObject]
		{
			HandleStateChanged(in_UEJsonObject);
		});
	});

	TSharedRef<FJsonObject> Options = MakeShareable(new FJsonObject());
	Options->SetNumberField(WwiseWaapiHelper::TRANSPORT, in_transportID);

	// Subscribe to object renamed-created-deleted and removed notifications.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		TSharedPtr<FJsonObject> outJsonResult;
		uint64 subscriptionID = 0;
		waapiClient->Subscribe(ak::wwise::core::transport::stateChanged, Options, wampEventCallback, subscriptionID, outJsonResult);
		return subscriptionID;
	}

	return 0;
}

void SWaapiPicker::HandlePlayWwiseItemCommandExecute()
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();

	// Loop to play all selected items.
	for (int32 i = 0; i < SelectedItems.Num(); ++i)
	{
		const FGuid& ItemId = SelectedItems[i]->ItemId;
		int32 transportID = -1;
		if (ItemToTransport.Contains(ItemId))
		{
			transportID = ItemToTransport[ItemId].TransportID;
		}
		else
		{
			transportID = CreateTransport(ItemId);
		}

		TogglePlayStop(transportID);
	}
}

void SWaapiPicker::StopAndDestroyAllTransports()
{
	for (auto iter = ItemToTransport.CreateIterator(); iter; ++iter)
	{
		StopTransport(iter->Value.TransportID);
		DestroyTransport(iter->Key);
	}
	ItemToTransport.Empty();
}

bool SWaapiPicker::HandleDeleteWwiseItemCommandCanExecute() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	if ((SelectedItems.Num() > 0) && !(TreeViewPtr->IsItemSelected(RootItems[EWwiseTreeItemType::Event]) || TreeViewPtr->IsItemSelected(RootItems[EWwiseTreeItemType::AuxBus]) || TreeViewPtr->IsItemSelected(RootItems[EWwiseTreeItemType::ActorMixer]) || TreeViewPtr->IsItemSelected(RootItems[EWwiseTreeItemType::AcousticTexture])))
	{
		for (int32 i = 0; i < SelectedItems.Num(); ++i)
		{
			if ((SelectedItems[i]->ItemType == EWwiseTreeItemType::PhysicalFolder) || (SelectedItems[i]->ItemType == EWwiseTreeItemType::StandaloneWorkUnit) || (SelectedItems[i]->ItemType == EWwiseTreeItemType::NestedWorkUnit))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void SWaapiPicker::HandleDeleteWwiseItemCommandExecute()
{
	TSharedPtr<FJsonObject> getResult;
	SWaapiPickerRow::CallWaapiExecuteUri(ak::wwise::core::undo::beginGroup, {}, getResult);
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	for (int32 i = 0; i < SelectedItems.Num(); ++i)
	{
		const FGuid& in_ItemId = SelectedItems[i]->ItemId;
		const FString itemIdStringField = in_ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces);
		SWaapiPickerRow::CallWaapiExecuteUri(ak::wwise::core::object::delete_, { {WwiseWaapiHelper::OBJECT, itemIdStringField} }, getResult);
	}
	SWaapiPickerRow::CallWaapiExecuteUri(ak::wwise::core::undo::endGroup, { {WwiseWaapiHelper::DISPLAY_NAME, WwiseWaapiHelper::DELETE_ITEMS} }, getResult);
	OnPopulateClicked();
}

void SWaapiPicker::HandleExploreWwiseItemCommandExecute() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	if (SelectedItems.Num())
	{
		// Construct the arguments Json object : Getting infos "from - a specific id/path"
		TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
		{
			TSharedPtr<FJsonObject> from = MakeShareable(new FJsonObject());
			TArray<TSharedPtr<FJsonValue>> fromJsonArray;
			fromJsonArray.Add(MakeShareable(new FJsonValueString(SelectedItems[0]->FolderPath)));
			from->SetArrayField(WwiseWaapiHelper::PATH, fromJsonArray);
			args->SetObjectField(WwiseWaapiHelper::FROM, from);
		}
		// Construct the options Json object : Getting specific infos to construct the wwise tree item "id - name - type - childrenCount - workunit:type - path - parent"
		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		{
			TArray<TSharedPtr<FJsonValue>> StructJsonArray;
			StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::FILEPATH)));
			options->SetArrayField(WwiseWaapiHelper::RETURN, StructJsonArray);
		}

		FString Path;
		// Connect to Wwise Authoring on localhost.
		FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
		if (waapiClient)
		{
			TSharedPtr<FJsonObject> outJsonResult;
			// Request data from Wwise using WAAPI
			if (waapiClient->Call(ak::wwise::core::object::get, args, options, outJsonResult))
			{
				// Recover the information from the Json object getResult and use it to get the item id.
				TArray<TSharedPtr<FJsonValue>> StructJsonArray = outJsonResult->GetArrayField(WwiseWaapiHelper::RETURN);
				const TSharedPtr<FJsonObject>& ItemInfoObj = StructJsonArray[0]->AsObject();
				Path = ItemInfoObj->GetStringField(WwiseWaapiHelper::FILEPATH);
			}
			else
			{
				UE_LOG(LogAkAudioPicker, Log, TEXT("Call Failed"));
				return;
			}
		}
		else
		{
			UE_LOG(LogAkAudioPicker, Log, TEXT("Unable to connect to localhost"));
			return;
		}
		FPlatformProcess::ExploreFolder(*Path);
	}
}

bool SWaapiPicker::HandleWwiseCommandCanExecute() const
{
	return (TreeViewPtr->GetSelectedItems().Num() == 1);
}

void SWaapiPicker::HandleFindWwiseItemInProjectExplorerCommandExecute() const
{
	const TArray<TSharedPtr<FWwiseTreeItem>> SelectedItems = TreeViewPtr->GetSelectedItems();
	if (SelectedItems.Num())
	{
		const FGuid& in_ItemId = SelectedItems[0]->ItemId;
		const FString& itemIdStringField = in_ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces);
		TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
		{
			args->SetStringField(WwiseWaapiHelper::COMMAND, WwiseWaapiHelper::FIND_IN_PROJECT_EXPLORER);
			TArray<TSharedPtr<FJsonValue>> fromJsonArray;
			fromJsonArray.Add(MakeShareable(new FJsonValueString(itemIdStringField)));
			args->SetArrayField(WwiseWaapiHelper::OBJECTS, fromJsonArray);
		}

		// Construct the options Json object;
		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());

		// Connect to Wwise Authoring on localhost.
		FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
		if (waapiClient)
		{
			TSharedPtr<FJsonObject> getResult;
			// Request changes in Wwise using WAAPI
			if (!waapiClient->Call(ak::wwise::ui::commands::execute, args, options, getResult))
			{
				UE_LOG(LogAkAudioPicker, Log, TEXT("Call Failed"));
			}
		}
		else
		{
			UE_LOG(LogAkAudioPicker, Log, TEXT("Unable to connect to localhost"));
		}
	}
}

void SWaapiPicker::HandleRefreshWaapiPickerCommandExecute()
{
	OnPopulateClicked();
}

void SWaapiPicker::HandleUndoWaapiPickerCommandExecute() const
{
	TSharedPtr<FJsonObject> getResult;
	SWaapiPickerRow::CallWaapiExecuteUri(ak::wwise::ui::commands::execute, { {WwiseWaapiHelper::COMMAND, WwiseWaapiHelper::UNDO} }, getResult);
}

void SWaapiPicker::HandleRedoWaapiPickerCommandExecute() const
{
	TSharedPtr<FJsonObject> getResult;
	SWaapiPickerRow::CallWaapiExecuteUri(ak::wwise::ui::commands::execute, { {WwiseWaapiHelper::COMMAND, WwiseWaapiHelper::REDO} }, getResult);
}

FReply SWaapiPicker::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyboardEvent)
{
	const FKey KeyPressed = InKeyboardEvent.GetKey();

	if ((KeyPressed == EKeys::SpaceBar))
	{
		// Play the wwise item.
		if (HandlePlayWwiseItemCommandCanExecute())
		{
			HandlePlayWwiseItemCommandExecute();
			return FReply::Handled();
		}
	}
	else if (KeyPressed == EKeys::F2)
	{
		// Rename key : Rename selected Wwise item.
		if (HandleRenameWwiseItemCommandCanExecute())
		{
			HandleRenameWwiseItemCommandExecute();
			return FReply::Handled();
		}
	}
	else if (KeyPressed == EKeys::Delete)
	{
		// Delete key : Delete selected Wwise item(s).
		if (HandleDeleteWwiseItemCommandCanExecute())
		{
			HandleDeleteWwiseItemCommandExecute();
			return FReply::Handled();
		}
	}
	else if (KeyPressed == EKeys::F5)
	{	// Populates the Waapi Picker.
		HandleRefreshWaapiPickerCommandExecute();
		return FReply::Handled();
	}
	else if ((KeyPressed == EKeys::Z) && InKeyboardEvent.IsControlDown())
	{
		// Undo
		HandleUndoWaapiPickerCommandExecute();
		return FReply::Handled();
	}
	else if ((KeyPressed == EKeys::Y) && InKeyboardEvent.IsControlDown())
	{
		// Redo
		HandleRedoWaapiPickerCommandExecute();
		return FReply::Handled();
	}
	else if (!bRestrictContextMenu && (KeyPressed == EKeys::One) && InKeyboardEvent.IsControlDown() && InKeyboardEvent.IsShiftDown())
	{
		// Finds the specified object in the Project Explorer (Sync Group 1).
		if (HandleWwiseCommandCanExecute())
		{
			HandleFindWwiseItemInProjectExplorerCommandExecute();
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

const TArray<TSharedPtr<FWwiseTreeItem>> SWaapiPicker::GetSelectedItems() const
{
	 return TreeViewPtr->GetSelectedItems();
}

const FString SWaapiPicker::GetSearchText() const
{
	return SearchBoxFilter->GetRawFilterText().ToString();
}

const void SWaapiPicker::SetSearchText(const FString& newText)
{
	SearchBoxPtr->SetText(FText::FromString(newText));
}

EActiveTimerReturnType SWaapiPicker::SetFocusPostConstruct(double InCurrentTime, float InDeltaTime)
{
	FWidgetPath WidgetToFocusPath;
	FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchBoxPtr.ToSharedRef(), WidgetToFocusPath);
	FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);

	return EActiveTimerReturnType::Stop;
}
#undef LOCTEXT_NAMESPACE