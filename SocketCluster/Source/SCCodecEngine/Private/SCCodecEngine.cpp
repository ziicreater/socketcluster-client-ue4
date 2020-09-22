// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCCodecEngine.h"
#include "SCCodecEngineModule.h"
#include "SCJsonConvert.h"

FString USCCodecEngine::Encode(TSharedPtr<FJsonValue> Data)
{
	return FString();
}

TSharedPtr<FJsonValue> USCCodecEngine::Decode(FString Data)
{
	return MakeShareable(new FJsonValueNull);
}
