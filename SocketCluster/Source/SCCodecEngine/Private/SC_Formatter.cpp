// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SC_Formatter.h"


FString USC_Formatter::encode(USCJsonObject* object)
{
	return object->EncodeJson();
}

USCJsonObject* USC_Formatter::decode(const FString& input)
{
	USCJsonObject* object = NewObject<USCJsonObject>();
	if (!input.IsEmpty() && object->DecodeJson(input))
	{
		return object;
	}
	else
	{
		return nullptr;
	}
}
