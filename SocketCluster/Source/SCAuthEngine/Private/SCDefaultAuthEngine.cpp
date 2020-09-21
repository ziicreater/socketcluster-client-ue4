// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCDefaultAuthEngine.h"
#include "Async/Async.h"

FString USCDefaultAuthEngine::saveToken(FString name, FString token)
{

#if !UE_BUILD_SHIPPING
	UE_LOG(SCAuthEngine, Log, TEXT("Saving Token : Name : %s Token : %s"), *name, *token);
#endif

	auto Promise = Async(EAsyncExecution::TaskGraph, [&] {
		localStorage.Add(name, token);
		return localStorage.FindRef(name);
	});
	return Promise.Get();
}

FString USCDefaultAuthEngine::removeToken(FString name)
{
#if !UE_BUILD_SHIPPING
	UE_LOG(SCAuthEngine, Log, TEXT("Removing Token : Name : %s"), *name);
#endif

	FString token = loadToken(name);

	localStorage.Remove(name);

	return token;
}

FString USCDefaultAuthEngine::loadToken(FString name)
{
#if !UE_BUILD_SHIPPING
	UE_LOG(SCAuthEngine, Log, TEXT("Loading Token : Name : %s"), *name);
#endif

	auto Promise = Async(EAsyncExecution::TaskGraph, [&] {
		return localStorage.FindRef(name);
	});
	return Promise.Get();
}
