// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCCodecEngine.h"
#include "SCCodecEngineModule.h"

FString USCCodecEngine::encode(TSharedPtr<FJsonValue> object)
{
	#if !UE_BUILD_SHIPPING
		UE_LOG(LogSCCodecEngine, Error, TEXT("USCCodecEngine::Encode has not been implemented!"));
	#endif
	return FString();
}

TSharedPtr<FJsonValue> USCCodecEngine::decode(const FString& input)
{
	#if !UE_BUILD_SHIPPING
		UE_LOG(LogSCCodecEngine, Error, TEXT("USCCodecEngine::Decode has not been implemented!"));
	#endif
	return nullptr;
}
