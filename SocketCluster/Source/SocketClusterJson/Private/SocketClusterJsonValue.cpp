// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SocketClusterJsonValue.h"
#include "SocketClusterJsonObject.h"
#include "SocketClusterJsonModule.h"

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

USocketClusterJsonValue* USocketClusterJsonValue::ConstructJsonValueNumber(UObject* WorldContextObject, float Number)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueNumber(Number));

	USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USocketClusterJsonValue* USocketClusterJsonValue::ConstructJsonValueString(UObject* WorldContextObject, const FString& StringValue)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueString(StringValue));

	USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USocketClusterJsonValue* USocketClusterJsonValue::ConstructJsonValueBool(UObject* WorldContextObject, bool InValue)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueBoolean(InValue));

	USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USocketClusterJsonValue* USocketClusterJsonValue::ConstructJsonValueArray(UObject* WorldContextObject, const TArray<USocketClusterJsonValue*>& InArray)
{
	// Prepare data array to create new value
	TArray< TSharedPtr<FJsonValue> > ValueArray;
	for (auto InVal : InArray)
	{
		ValueArray.Add(InVal->GetRootValue());
	}

	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueArray(ValueArray));

	USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USocketClusterJsonValue* USocketClusterJsonValue::ConstructJsonValueObject(UObject* WorldContextObject, USocketClusterJsonObject *JsonObject)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueObject(JsonObject->GetRootObject()));

	USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

USocketClusterJsonValue* ConstructJsonValue(UObject* WorldContextObject, const TSharedPtr<FJsonValue>& InValue)
{
	TSharedPtr<FJsonValue> NewVal = InValue;

	USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

TSharedPtr<FJsonValue>& USocketClusterJsonValue::GetRootValue()
{
	return JsonVal;
}

void USocketClusterJsonValue::SetRootValue(TSharedPtr<FJsonValue>& JsonValue)
{
	JsonVal = JsonValue;
}


//////////////////////////////////////////////////////////////////////////
// FJsonValue API

EVaJson::Type USocketClusterJsonValue::GetType() const
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

FString USocketClusterJsonValue::GetTypeString() const
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

bool USocketClusterJsonValue::IsNull() const
{
	if (!JsonVal.IsValid())
	{
		return true;
	}

	return JsonVal->IsNull();
}

float USocketClusterJsonValue::AsNumber() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}

	return JsonVal->AsNumber();
}

FString USocketClusterJsonValue::AsString() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("String"));
		return FString();
	}

	return JsonVal->AsString();
}

bool USocketClusterJsonValue::AsBool() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Boolean"));
		return false;
	}

	return JsonVal->AsBool();
}

TArray<USocketClusterJsonValue*> USocketClusterJsonValue::AsArray() const
{
	TArray<USocketClusterJsonValue*> OutArray;

	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Array"));
		return OutArray;
	}

	TArray< TSharedPtr<FJsonValue> > ValArray = JsonVal->AsArray();
	for (auto Value : ValArray)
	{
		USocketClusterJsonValue* NewValue = NewObject<USocketClusterJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

USocketClusterJsonObject* USocketClusterJsonValue::AsObject()
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Object"));
		return nullptr;
	}

	TSharedPtr<FJsonObject> NewObj = JsonVal->AsObject();

	USocketClusterJsonObject* JsonObj = NewObject<USocketClusterJsonObject>();
	JsonObj->SetRootObject(NewObj);

	return JsonObj;
}


//////////////////////////////////////////////////////////////////////////
// Helpers

void USocketClusterJsonValue::ErrorMessage(const FString& InType) const
{
	UE_LOG(LogSocketClusterJson, Error, TEXT("Json Value of type '%s' used as a '%s'."), *GetTypeString(), *InType);
}