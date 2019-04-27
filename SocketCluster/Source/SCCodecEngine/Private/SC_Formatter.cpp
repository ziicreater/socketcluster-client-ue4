// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SC_Formatter.h"


FString USC_Formatter::encode(TSharedPtr<FJsonValue> object)
{
	return USCJsonConvert::ToJsonString(object);
}

TSharedPtr<FJsonValue> USC_Formatter::decode(const FString& input)
{
	TSharedPtr<FJsonValue> JsonValue = USCJsonConvert::JsonStringToJsonValue(input);
	return JsonValue;
}
