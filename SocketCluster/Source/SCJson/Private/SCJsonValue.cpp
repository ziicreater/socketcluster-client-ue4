// Modifications Copyright 2019 ZiiCreater, LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved
// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SCJsonValue.h"
#include "SCJsonConvert.h"
#include "SCJsonModule.h"

#if PLATFORM_WINDOWS
#pragma region FJsonValueBinary
#endif


TArray<uint8> FJsonValueBinary::AsBinary(const TSharedPtr<FJsonValue>& InJsonValue)
{
	if (FJsonValueBinary::IsBinary(InJsonValue))
	{
		TSharedPtr<FJsonValueBinary> BinaryValue = StaticCastSharedPtr<FJsonValueBinary>(InJsonValue);
		return BinaryValue->AsBinary();
	}
	else
	{
		TArray<uint8> EmptyArray;
		return EmptyArray;
	}
}


bool FJsonValueBinary::IsBinary(const TSharedPtr<FJsonValue>& InJsonValue)
{
	//use our hackery to determine if we got a binary string
	bool IgnoreBool;
	return !InJsonValue->TryGetBool(IgnoreBool);
}

#if PLATFORM_WINDOWS
#pragma endregion FJsonValueBinary
#pragma region USCJsonValue
#endif

USCJsonValue::USCJsonValue(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

USCJsonValue* USCJsonValue::ConstructJsonValueNumber(UObject* WorldContextObject, float Number)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueNumber(Number));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USCJsonValue* USCJsonValue::ConstructJsonValueString(UObject* WorldContextObject, const FString& StringValue)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueString(StringValue));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USCJsonValue* USCJsonValue::ConstructJsonValueBool(UObject* WorldContextObject, bool InValue)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueBoolean(InValue));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USCJsonValue* USCJsonValue::ConstructJsonValueArray(UObject* WorldContextObject, const TArray<USCJsonValue*>& InArray)
{
	// Prepare data array to create new value
	TArray< TSharedPtr<FJsonValue> > ValueArray;
	for (auto InVal : InArray)
	{
		ValueArray.Add(InVal->GetRootValue());
	}

	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueArray(ValueArray));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USCJsonValue* USCJsonValue::ConstructJsonValueObject(USCJsonObject *JsonObject, UObject* WorldContextObject)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueObject(JsonObject->GetRootObject()));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}


USCJsonValue* USCJsonValue::ConstructJsonValueBinary(UObject* WorldContextObject, TArray<uint8> ByteArray)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueBinary(ByteArray));

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USCJsonValue* ConstructJsonValue(UObject* WorldContextObject, const TSharedPtr<FJsonValue>& InValue)
{
	TSharedPtr<FJsonValue> NewVal = InValue;

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}


USCJsonValue* USCJsonValue::ValueFromJsonString(UObject* WorldContextObject, const FString& StringValue)
{
	TSharedPtr<FJsonValue> NewVal = USCJsonConvert::JsonStringToJsonValue(StringValue);

	USCJsonValue* NewValue = NewObject<USCJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

TSharedPtr<FJsonValue>& USCJsonValue::GetRootValue()
{
	return JsonVal;
}

void USCJsonValue::SetRootValue(TSharedPtr<FJsonValue>& JsonValue)
{
	JsonVal = JsonValue;
}


//////////////////////////////////////////////////////////////////////////
// FJsonValue API

ESCJson::Type USCJsonValue::GetType() const
{
	if (!JsonVal.IsValid())
	{
		return ESCJson::None;
	}

	switch (JsonVal->Type)
	{
	case EJson::None:
		return ESCJson::None;

	case EJson::Null:
		return ESCJson::Null;

	case EJson::String:
		if (FJsonValueBinary::IsBinary(JsonVal))
		{
			return ESCJson::Binary;
		}
		else
		{
			return ESCJson::String;
		}
	case EJson::Number:
		return ESCJson::Number;

	case EJson::Boolean:
		return ESCJson::Boolean;

	case EJson::Array:
		return ESCJson::Array;

	case EJson::Object:
		return ESCJson::Object;

	default:
		return ESCJson::None;
	}
}

FString USCJsonValue::GetTypeString() const
{
	if (!JsonVal.IsValid())
	{
		return "None";
	}

	switch (JsonVal->Type)
	{
	case EJson::None:
		return TEXT("None");

	case EJson::Null:
		return TEXT("Null");

	case EJson::String:
		return TEXT("String");

	case EJson::Number:
		return TEXT("Number");

	case EJson::Boolean:
		return TEXT("Boolean");

	case EJson::Array:
		return TEXT("Array");

	case EJson::Object:
		return TEXT("Object");

	default:
		return TEXT("None");
	}
}

bool USCJsonValue::IsNull() const 
{
	if (!JsonVal.IsValid())
	{
		return true;
	}

	return JsonVal->IsNull();
}

float USCJsonValue::AsNumber() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}

	return JsonVal->AsNumber();
}

FString USCJsonValue::AsString() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("String"));
		return FString();
	}

	//Auto-convert non-strings instead of getting directly
	if (JsonVal->Type != EJson::String)
	{
		return EncodeJson();
	}
	else
	{
		return JsonVal->AsString();
	}
}

bool USCJsonValue::AsBool() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Boolean"));
		return false;
	}

	return JsonVal->AsBool();
}

TArray<USCJsonValue*> USCJsonValue::AsArray() const
{
	TArray<USCJsonValue*> OutArray;

	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Array"));
		return OutArray;
	}

	TArray< TSharedPtr<FJsonValue> > ValArray = JsonVal->AsArray();
	for (auto Value : ValArray)
	{
		USCJsonValue* NewValue = NewObject<USCJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

USCJsonObject* USCJsonValue::AsObject()
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Object"));
		return nullptr;
	}

	TSharedPtr<FJsonObject> NewObj = JsonVal->AsObject();

	USCJsonObject* JsonObj = NewObject<USCJsonObject>();
	JsonObj->SetRootObject(NewObj);

	return JsonObj;
}


TArray<uint8> USCJsonValue::AsBinary()
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Binary"));
		TArray<uint8> ByteArray;
		return ByteArray;
	}
	
	//binary object pretending & starts with non-json format? it's our disguise binary
	if (JsonVal->Type == EJson::String)
	{
		//it's a legit binary
		if (FJsonValueBinary::IsBinary(JsonVal))
		{
			//Valid binary available
			return FJsonValueBinary::AsBinary(JsonVal);
		}

		//It's a string, decode as if hex encoded binary
		else
		{
			const FString& HexString = JsonVal->AsString();

			TArray<uint8> ByteArray;
			ByteArray.AddUninitialized(HexString.Len() / 2);

			bool DidConvert = FString::ToHexBlob(HexString, ByteArray.GetData(), ByteArray.Num());
			
			//Empty our array if conversion failed
			if (!DidConvert)
			{
				ByteArray.Empty();
			}
			return ByteArray;
		}
	}
	//Not a binary nor binary string, return empty array
	else
	{
		//Empty array
		TArray<uint8> ByteArray;
		return ByteArray;
	}
}

FString USCJsonValue::EncodeJson() const
{ 
	return USCJsonConvert::ToJsonString(JsonVal);
}

//////////////////////////////////////////////////////////////////////////
// Helpers

void USCJsonValue::ErrorMessage(const FString& InType) const
{
	UE_LOG(LogSCJson, Error, TEXT("Json Value of type '%s' used as a '%s'."), *GetTypeString(), *InType);
}

#if PLATFORM_WINDOWS
#pragma endregion USCJsonValue
#endif