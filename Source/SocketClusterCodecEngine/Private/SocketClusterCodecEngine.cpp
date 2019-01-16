// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#include "SocketClusterCodecEngine.h"
#include "SocketClusterCodecEngineModule.h"

FString ISocketClusterCodecEngine::Encode(USocketClusterJsonObject* input)
{
	return FString();
}

USocketClusterJsonObject* ISocketClusterCodecEngine::Decode(FString input)
{
	return nullptr;
}
