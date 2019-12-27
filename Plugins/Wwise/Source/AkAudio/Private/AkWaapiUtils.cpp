// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
 includes.
------------------------------------------------------------------------------------*/
#include "AkWaapiUtils.h"
#include "AkAudioDevice.h"
#include "AkWaapiClient.h"

/*------------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------------*/
#define LOCTEXT_NAMESPACE "AkAudio"

DEFINE_LOG_CATEGORY(LogAkWaapiUtils);

/*------------------------------------------------------------------------------------
Statics and Globals
------------------------------------------------------------------------------------*/
const FString WwiseWaapiHelper::BACK_SLASH					 = TEXT("\\");
const FString WwiseWaapiHelper::NEW							 = TEXT("new");
const FString WwiseWaapiHelper::AT							 = TEXT("@");
const FString WwiseWaapiHelper::ID                           = TEXT("id");
const FString WwiseWaapiHelper::RETURN                       = TEXT("return");
const FString WwiseWaapiHelper::PATH                         = TEXT("path");
const FString WwiseWaapiHelper::FILEPATH                     = TEXT("filePath");
const FString WwiseWaapiHelper::FROM                         = TEXT("from");
const FString WwiseWaapiHelper::NAME						 = TEXT("name");
const FString WwiseWaapiHelper::NEW_NAME					 = TEXT("newName");
const FString WwiseWaapiHelper::TYPE                         = TEXT("type");
const FString WwiseWaapiHelper::CHILDREN                     = TEXT("children");
const FString WwiseWaapiHelper::CHILDREN_COUNT               = TEXT("childrenCount");
const FString WwiseWaapiHelper::ANCESTORS                    = TEXT("ancestors"); 
const FString WwiseWaapiHelper::DESCENDANTS                  = TEXT("descendants"); 
const FString WwiseWaapiHelper::WOKUNIT_TYPE                 = TEXT("workunit:type");
const FString WwiseWaapiHelper::FOLDER                       = TEXT("Folder");
const FString WwiseWaapiHelper::PHYSICAL_FOLDER              = TEXT("PhysicalFolder");
const FString WwiseWaapiHelper::SEARCH						 = TEXT("search");
const FString WwiseWaapiHelper::WHERE						 = TEXT("where");
const FString WwiseWaapiHelper::NAMECONTAINS				 = TEXT("name:contains");
const FString WwiseWaapiHelper::RANGE						 = TEXT("range");
const FString WwiseWaapiHelper::PARENT                       = TEXT("parent");
const FString WwiseWaapiHelper::SELECT                       = TEXT("select");
const FString WwiseWaapiHelper::TRANSFORM                    = TEXT("transform");
const FString WwiseWaapiHelper::OBJECT                       = TEXT("object");
const FString WwiseWaapiHelper::OBJECTS                      = TEXT("objects");
const FString WwiseWaapiHelper::VALUE                        = TEXT("value");
const FString WwiseWaapiHelper::COMMAND                      = TEXT("command");
const FString WwiseWaapiHelper::TRANSPORT                    = TEXT("transport");
const FString WwiseWaapiHelper::ACTION                       = TEXT("action");
const FString WwiseWaapiHelper::PLAY                         = TEXT("play");
const FString WwiseWaapiHelper::STOP                         = TEXT("stop");
const FString WwiseWaapiHelper::PLAYSTOP                     = TEXT("playStop");
const FString WwiseWaapiHelper::STOPPED                      = TEXT("stopped");
const FString WwiseWaapiHelper::PLAYING                      = TEXT("playing");
const FString WwiseWaapiHelper::DISPLAY_NAME                 = TEXT("displayName");
const FString WwiseWaapiHelper::DELETE_ITEMS                 = TEXT("Delete Items");
const FString WwiseWaapiHelper::DRAG_DROP_ITEMS              = TEXT("Drag Drop Items");
const FString WwiseWaapiHelper::UNDO                         = TEXT("Undo");
const FString WwiseWaapiHelper::REDO                         = TEXT("Redo");
const FString WwiseWaapiHelper::STATE                        = TEXT("state");
const FString WwiseWaapiHelper::OF_TYPE                      = TEXT("ofType");
const FString WwiseWaapiHelper::PROJECT                      = TEXT("Project");
const FString WwiseWaapiHelper::PROPERTY                     = TEXT("property");
const FString WwiseWaapiHelper::VOLUME						 = TEXT("Volume");
const FString WwiseWaapiHelper::CLASSID						 = TEXT("classId");
const FString WwiseWaapiHelper::FIND_IN_PROJECT_EXPLORER     = TEXT("FindInProjectExplorerSyncGroup1");
const FString WwiseWaapiHelper::RESTRICTION                  = TEXT("restriction");
const FString WwiseWaapiHelper::UI                           = TEXT("ui");
const FString WwiseWaapiHelper::MIN                          = TEXT("min");
const FString WwiseWaapiHelper::MAX                          = TEXT("max");

/*------------------------------------------------------------------------------------
 Methods
------------------------------------------------------------------------------------*/

const bool CallWappiGetPropertySate(const FString& ItemID, const FString& ItemProperty, TSharedPtr<FJsonObject>& ItemInfoResult)
{
	// Construct the arguments Json object.
	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	{
		TSharedPtr<FJsonObject> from = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> fromJsonArray;
		fromJsonArray.Add(MakeShareable(new FJsonValueString(ItemID)));
		from->SetArrayField(WwiseWaapiHelper::ID, fromJsonArray);
		args->SetObjectField(WwiseWaapiHelper::FROM, from);
	}
	
	// Construct the options Json object.
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	{
		TArray<TSharedPtr<FJsonValue>> StructJsonArray;
		StructJsonArray.Add(MakeShareable(new FJsonValueString(WwiseWaapiHelper::AT + ItemProperty)));
		options->SetArrayField(WwiseWaapiHelper::RETURN, StructJsonArray);
	}
	
	TSharedPtr<FJsonObject> outJsonResult;
	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		// Request data from Wwise using WAAPI
		if (waapiClient->Call(ak::wwise::core::object::get, args, options, outJsonResult))
		{
			// Recover the information from the Json object outJsonResult and use it to get the property state.
			TArray<TSharedPtr<FJsonValue>> StructJsonArray = outJsonResult->GetArrayField(WwiseWaapiHelper::RETURN);
			ItemInfoResult = StructJsonArray[0]->AsObject();
			return true;
		}
	}

	return false;
}

const bool SubscribeToPropertyStateChange(const FString& ItemID, const FString& ItemProperty, WampEventCallback CallBack, uint64& SubscriptionId, TSharedPtr<FJsonObject>& outJsonResult)
{
	// Connect to Wwise Authoring on localhost.
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient)
	{
		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());


		options->SetStringField(WwiseWaapiHelper::OBJECT, ItemID);
		options->SetStringField(WwiseWaapiHelper::PROPERTY, ItemProperty);

		// Subscribe to action notifications.
		return waapiClient->Subscribe(ak::wwise::core::object::propertyChanged, options, CallBack, SubscriptionId, outJsonResult);
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
