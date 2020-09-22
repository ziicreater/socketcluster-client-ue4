// Modifications Copyright 2020-current ZiiCreater LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved


// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "SCJsonLibrary.h"
#include "SCJsonModule.h"
#include "Json/Public/Dom/JsonObject.h"
#include "Json/Public/Dom/JsonValue.h"
#include "SCJsonValue.h"
#include "SCJsonObject.h"
#include "Misc/Base64.h"
#include "Engine/Engine.h"

//////////////////////////////////////////////////////////////////////////
// Helpers

FString USCJsonLibrary::PercentEncode(const FString& Source)
{
	FString OutText = Source;

	OutText = OutText.Replace(TEXT(" "), TEXT("%20"));
	OutText = OutText.Replace(TEXT("!"), TEXT("%21"));
	OutText = OutText.Replace(TEXT("\""), TEXT("%22"));
	OutText = OutText.Replace(TEXT("#"), TEXT("%23"));
	OutText = OutText.Replace(TEXT("$"), TEXT("%24"));
	OutText = OutText.Replace(TEXT("&"), TEXT("%26"));
	OutText = OutText.Replace(TEXT("'"), TEXT("%27"));
	OutText = OutText.Replace(TEXT("("), TEXT("%28"));
	OutText = OutText.Replace(TEXT(")"), TEXT("%29"));
	OutText = OutText.Replace(TEXT("*"), TEXT("%2A"));
	OutText = OutText.Replace(TEXT("+"), TEXT("%2B"));
	OutText = OutText.Replace(TEXT(","), TEXT("%2C"));
	OutText = OutText.Replace(TEXT("/"), TEXT("%2F"));
	OutText = OutText.Replace(TEXT(":"), TEXT("%3A"));
	OutText = OutText.Replace(TEXT(";"), TEXT("%3B"));
	OutText = OutText.Replace(TEXT("="), TEXT("%3D"));
	OutText = OutText.Replace(TEXT("?"), TEXT("%3F"));
	OutText = OutText.Replace(TEXT("@"), TEXT("%40"));
	OutText = OutText.Replace(TEXT("["), TEXT("%5B"));
	OutText = OutText.Replace(TEXT("]"), TEXT("%5D"));
	OutText = OutText.Replace(TEXT("{"), TEXT("%7B"));
	OutText = OutText.Replace(TEXT("}"), TEXT("%7D"));

	return OutText;
}

FString USCJsonLibrary::Base64Encode(const FString& Source)
{
	return FBase64::Encode(Source);
}

FString USCJsonLibrary::Base64EncodeBytes(const TArray<uint8>& Source)
{
	return FBase64::Encode(Source);
}

bool USCJsonLibrary::Base64Decode(const FString& Source, FString& Dest)
{
	return FBase64::Decode(Source, Dest);
}


bool USCJsonLibrary::Base64DecodeBytes(const FString& Source, TArray<uint8>& Dest)
{
	return FBase64::Decode(Source, Dest);
}

bool USCJsonLibrary::StringToJsonValueArray(const FString& JsonString, TArray<USCJsonValue*>& OutJsonValueArray)
{
	TArray < TSharedPtr<FJsonValue>> RawJsonValueArray;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(*JsonString);
	FJsonSerializer::Deserialize(Reader, RawJsonValueArray);

	for (auto Value : RawJsonValueArray)
	{
		auto SJsonValue = NewObject<USCJsonValue>();
		SJsonValue->SetRootValue(Value);
		OutJsonValueArray.Add(SJsonValue);
	}

	return OutJsonValueArray.Num() > 0;
}


FString USCJsonLibrary::Conv_JsonObjectToString(USCJsonObject* InObject)
{
	if(InObject)
	{
		return InObject->EncodeJson();
	}

	return "";
}


USCJsonObject* USCJsonLibrary::Conv_JsonValueToJsonObject(class USCJsonValue* InValue)
{
	if(InValue)
	{
		return InValue->AsObject();
	}

	return nullptr;
}

USCJsonValue* USCJsonLibrary::Conv_ArrayToJsonValue(const TArray<USCJsonValue*>& InArray)
{
	return USCJsonValue::ConstructJsonValueArray(nullptr, InArray);
}


USCJsonValue* USCJsonLibrary::Conv_JsonObjectToJsonValue(USCJsonObject* InObject)
{
	return USCJsonValue::ConstructJsonValueObject(InObject, nullptr);
}


USCJsonValue* USCJsonLibrary::Conv_BytesToJsonValue(const TArray<uint8>& InBytes)
{
	return USCJsonValue::ConstructJsonValueBinary(nullptr, InBytes);
}


USCJsonValue* USCJsonLibrary::Conv_StringToJsonValue(const FString& InString)
{
	return USCJsonValue::ConstructJsonValueString(nullptr, InString);
}


USCJsonValue* USCJsonLibrary::Conv_IntToJsonValue(int32 InInt)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueNumber(InInt));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USCJsonValue* USCJsonLibrary::Conv_FloatToJsonValue(float InFloat)
{
	return USCJsonValue::ConstructJsonValueNumber(nullptr, InFloat);
}


USCJsonValue* USCJsonLibrary::Conv_BoolToJsonValue(bool InBool)
{
	return USCJsonValue::ConstructJsonValueBool(nullptr, InBool);
}


int32 USCJsonLibrary::Conv_JsonValueToInt(USCJsonValue* InValue)
{
	if(InValue)
	{
		return (int32)InValue->AsNumber();
	}

	return 0;
}


float USCJsonLibrary::Conv_JsonValueToFloat(USCJsonValue* InValue)
{
	if (InValue)
	{
		return InValue->AsNumber();
	}

	return 0.f;
}


bool USCJsonLibrary::Conv_JsonValueToBool(USCJsonValue* InValue)
{
	if (InValue)
	{
		return InValue->AsBool();
	}

	return false;
}


TArray<uint8> USCJsonLibrary::Conv_JsonValueToBytes(USCJsonValue* InValue)
{
	if (InValue)
	{
		return InValue->AsBinary();
	}

	return TArray<uint8>();
}

