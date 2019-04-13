// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SCJsonValue.h"
#include "SCJsonObject.h"
#include "SCJsonModule.h"

#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MINOR_VERSION >= 15
#include "CoreMinimal.h"
#include "EngineDefines.h"
#include "Engine/Engine.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#else
#include "CoreUObject.h"
#include "Engine.h"
#endif

#include "Json.h"

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

USCJsonValue* USCJsonValue::ConstructJsonValueObject(UObject* WorldContextObject, USCJsonObject *JsonObject)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueObject(JsonObject->GetRootObject()));

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

EVaJson::Type USCJsonValue::GetType() const
{
	if (!JsonVal.IsValid())
	{
		return EVaJson::None;
	}

	switch (JsonVal->Type)
	{
	case EJson::None:
		return EVaJson::None;

	case EJson::Null:
		return EVaJson::Null;

	case EJson::String:
		return EVaJson::String;

	case EJson::Number:
		return EVaJson::Number;

	case EJson::Boolean:
		return EVaJson::Boolean;

	case EJson::Array:
		return EVaJson::Array;

	case EJson::Object:
		return EVaJson::Object;

	default:
		return EVaJson::None;
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

	return JsonVal->AsString();
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


//////////////////////////////////////////////////////////////////////////
// Helpers

void USCJsonValue::ErrorMessage(const FString& InType) const
{
	UE_LOG(LogSCJson, Error, TEXT("Json Value of type '%s' used as a '%s'."), *GetTypeString(), *InType);
}