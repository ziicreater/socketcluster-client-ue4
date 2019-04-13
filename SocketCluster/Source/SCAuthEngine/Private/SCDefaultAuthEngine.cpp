// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCDefaultAuthEngine.h"
#include "SCAuthEngineModule.h"

void USCDefaultAuthEngine::saveToken(FString name, FString token, TFunction<void(USCJsonObject*, FString&)> callback)
{
	Storage.Add(name, token);

	callback(nullptr, token);
}

void USCDefaultAuthEngine::removeToken(FString name, TFunction<void(USCJsonObject*, FString&)> callback)
{
	FString token;

	this->loadToken(name, [&](USCJsonObject* error, FString& authToken) {
		token = authToken;
	});

	Storage.Remove(name);

	callback(nullptr, token);
}

void USCDefaultAuthEngine::loadToken(FString name, TFunction<void(USCJsonObject*, FString&)> callback)
{
	FString token;

	token = Storage.FindRef(name);

	callback(nullptr, token);
}
