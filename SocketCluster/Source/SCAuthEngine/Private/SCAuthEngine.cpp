// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCAuthEngine.h"
#include "Async/Async.h"
#include "SCAuthEngineModule.h"

FString USCAuthEngine::SaveToken(FString Name, FString Token)
{

#if !UE_BUILD_SHIPPING
	UE_LOG(SCAuthEngine, Log, TEXT("Saving Token : Name : %s Token : %s"), *Name, *Token);
#endif

	auto Promise = Async(EAsyncExecution::TaskGraph, [&] {
		localStorage.Add(Name, Token);
		return localStorage.FindRef(Name);
	});
	return Promise.Get();
}

FString USCAuthEngine::RemoveToken(FString Name)
{
#if !UE_BUILD_SHIPPING
	UE_LOG(SCAuthEngine, Log, TEXT("Removing Token : Name : %s"), *Name);
#endif

	FString token = LoadToken(Name);

	localStorage.Remove(Name);

	return token;
}

FString USCAuthEngine::LoadToken(FString Name)
{
#if !UE_BUILD_SHIPPING
	UE_LOG(SCAuthEngine, Log, TEXT("Loading Token : Name : %s"), *Name);
#endif

	auto Promise = Async(EAsyncExecution::TaskGraph, [&] {
		return localStorage.FindRef(Name);
	});
	return Promise.Get();
}