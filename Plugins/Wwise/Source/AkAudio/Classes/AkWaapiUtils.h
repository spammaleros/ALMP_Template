// Copyright (c) 2006-2017 Audiokinetic Inc. / All Rights Reserved

/*------------------------------------------------------------------------------------
	WwiseUtils.h
------------------------------------------------------------------------------------*/
#pragma once

#include "Dom/JsonObject.h"
#include "AkWaapiClient.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAkWaapiUtils, Log, All);

class WwiseWaapiHelper
{
public:
	static const FString BACK_SLASH;
	static const FString NEW;
	static const FString AT;
	static const FString ID;
	static const FString RETURN;
	static const FString PATH;
	static const FString FILEPATH;
	static const FString FROM;
	static const FString NAME;
	static const FString NEW_NAME;
	static const FString TYPE;
	static const FString CHILDREN;
	static const FString CHILDREN_COUNT;
	static const FString ANCESTORS;
	static const FString DESCENDANTS;
	static const FString WOKUNIT_TYPE;
	static const FString FOLDER;
	static const FString PHYSICAL_FOLDER;
	static const FString SEARCH;
	static const FString WHERE;
	static const FString NAMECONTAINS;
	static const FString RANGE;
	static const FString PARENT;
	static const FString SELECT;
	static const FString TRANSFORM;
	static const FString OBJECT;
	static const FString OBJECTS;
	static const FString VALUE;
	static const FString COMMAND;
	static const FString TRANSPORT;
	static const FString ACTION;
	static const FString PLAY;
	static const FString STOP;
	static const FString PLAYSTOP;
	static const FString STOPPED;
	static const FString PLAYING;
	static const FString DISPLAY_NAME;
	static const FString DELETE_ITEMS;
	static const FString DRAG_DROP_ITEMS;
	static const FString UNDO;
	static const FString REDO;
	static const FString STATE;
	static const FString OF_TYPE;
	static const FString PROJECT;
	static const FString PROPERTY;
	static const FString VOLUME;
	static const FString CLASSID;
	static const FString FIND_IN_PROJECT_EXPLORER;
	static const FString RESTRICTION;
    static const FString UI;
	static const FString MIN;
	static const FString MAX;
};

const bool CallWappiGetPropertySate(const FString& ItemID, const FString& ItemProperty, TSharedPtr<FJsonObject>& ItemInfoResult); 
const bool SubscribeToPropertyStateChange(const FString& ItemID, const FString& ItemProperty, WampEventCallback CallBack, uint64& SubscriptionId, TSharedPtr<FJsonObject>& outJsonResult);