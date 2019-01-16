// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#include "SC_Formatter.h"
#include "SocketClusterJsonObject.h"

FString USC_Formatter::Encode(USocketClusterJsonObject* input)
{
	return input->EncodeJson();
}

USocketClusterJsonObject* USC_Formatter::Decode(FString input)
{
	USocketClusterJsonObject* SocketClusterJsonObject = NewObject<USocketClusterJsonObject>();
	if (!input.IsEmpty() && SocketClusterJsonObject->DecodeJson(input))
	{
		return SocketClusterJsonObject;
	}
	else
	{
		return nullptr;
	}
}
