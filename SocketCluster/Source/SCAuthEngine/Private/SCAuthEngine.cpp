// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCAuthEngine.h"
#include "SCJsonObject.h"
#include "SCErrors.h"
#include "SCAuthEngineModule.h"

void USCAuthEngine::saveToken(FString name, FString token, TFunction<void(USCJsonObject*, FString&)> callback)
{
	USCJsonObject* err = USCErrors::InvalidActionError("saveToken function has not been implemented!");
	callback(err, token);
}

void USCAuthEngine::removeToken(FString name, TFunction<void(USCJsonObject*, FString&)> callback)
{
	USCJsonObject* err = USCErrors::InvalidActionError("removeToken function has not been implemented!");
	callback(err, name);
}

void USCAuthEngine::loadToken(FString name, TFunction<void(USCJsonObject*, FString&)> callback)
{
	USCJsonObject* err = USCErrors::InvalidActionError("loadToken function has not been implemented!");
	callback(err, name);
}
