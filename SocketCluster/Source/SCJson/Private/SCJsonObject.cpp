// Modifications Copyright 2019 ZiiCreater, LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved
// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SCJsonObject.h"
#include "Runtime/Json/Public/Policies/CondensedJsonPrintPolicy.h"
#include "Runtime/Json/Public/Serialization/JsonWriter.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"

typedef TJsonWriterFactory< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriterFactory;
typedef TJsonWriter< TCHAR, TCondensedJsonPrintPolicy<TCHAR> > FCondensedJsonStringWriter;

USCJsonObject::USCJsonObject(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Reset();
}

USCJsonObject* USCJsonObject::ConstructJsonObject(UObject* WorldContextObject)
{
	return NewObject<USCJsonObject>();
}

void USCJsonObject::Reset()
{
	if (JsonObj.IsValid())
	{
		JsonObj.Reset();
	}

	JsonObj = MakeShareable(new FJsonObject());
}

TSharedPtr<FJsonObject>& USCJsonObject::GetRootObject()
{
	return JsonObj;
}

void USCJsonObject::SetRootObject(const TSharedPtr<FJsonObject>& JsonObject)
{
	JsonObj = JsonObject;
}


//////////////////////////////////////////////////////////////////////////
// Serialization

FString USCJsonObject::EncodeJson() const
{
	if (!JsonObj.IsValid())
	{
		return TEXT("");
	}

	FString OutputString;
	TSharedRef< FCondensedJsonStringWriter > Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

	return OutputString;
}

FString USCJsonObject::EncodeJsonToSingleString() const
{
	FString OutputString = EncodeJson();

	// Remove line terminators
	OutputString.Replace(LINE_TERMINATOR, TEXT(""));
	
	// Remove tabs
	OutputString.Replace(LINE_TERMINATOR, TEXT("\t"));

	return OutputString;
}

bool USCJsonObject::DecodeJson(const FString& JsonString)
{
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(*JsonString);
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
	{
		return true;
	}

	// If we've failed to deserialize the string, we should clear our internal data
	Reset();

	UE_LOG(LogSCJson, Error, TEXT("Json decoding failed for: %s"), *JsonString);

	return false;
}


//////////////////////////////////////////////////////////////////////////
// FJsonObject API

TArray<FString> USCJsonObject::GetFieldNames()
{
	TArray<FString> Result;
	
	if (!JsonObj.IsValid())
	{
		return Result;
	}
	
	JsonObj->Values.GetKeys(Result);
	
	return Result;
}

bool USCJsonObject::HasField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return false;
	}

	return JsonObj->HasField(FieldName);
}

void USCJsonObject::RemoveField(const FString& FieldName)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->RemoveField(FieldName);
}

USCJsonValue* USCJsonObject::GetField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return nullptr;
	}

	TSharedPtr<FJsonValue> NewVal = JsonObj->TryGetField(FieldName);
	if (NewVal.IsValid())
	{
		USCJsonValue* NewValue = NewObject<USCJsonValue>();
		NewValue->SetRootValue(NewVal);

		return NewValue;
	}
	
	return nullptr;
}

void USCJsonObject::SetField(const FString& FieldName, USCJsonValue* JsonValue)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetField(FieldName, JsonValue->GetRootValue());
}


//////////////////////////////////////////////////////////////////////////
// FJsonObject API Helpers (easy to use with simple Json objects)

float USCJsonObject::GetNumberField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || !JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0.0f;
	}

	return JsonObj->GetNumberField(FieldName);
}

void USCJsonObject::SetNumberField(const FString& FieldName, float Number)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

int32 USCJsonObject::GetIntegerField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || !JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0;
	}
	return JsonObj->GetIntegerField(FieldName);
}

void USCJsonObject::SetIntegerField(const FString & FieldName, int32 Number)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}
	JsonObj->SetNumberField(FieldName, Number);
}

FString USCJsonObject::GetStringField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || !JsonObj->HasTypedField<EJson::String>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type String"), *FieldName);
		return TEXT("");
	}
		
	return JsonObj->GetStringField(FieldName);
}

void USCJsonObject::SetStringField(const FString& FieldName, const FString& StringValue)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetStringField(FieldName, StringValue);
}

bool USCJsonObject::GetBoolField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || !JsonObj->HasTypedField<EJson::Boolean>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Boolean"), *FieldName);
		return false;
	}

	return JsonObj->GetBoolField(FieldName);
}

void USCJsonObject::SetBoolField(const FString& FieldName, bool InValue)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetBoolField(FieldName, InValue);
}

TArray<USCJsonValue*> USCJsonObject::GetArrayField(const FString& FieldName)
{
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Array"), *FieldName);
	}

	TArray<USCJsonValue*> OutArray;
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return OutArray;
	}

	TArray< TSharedPtr<FJsonValue> > ValArray = JsonObj->GetArrayField(FieldName);
	for (auto Value : ValArray)
	{
		USCJsonValue* NewValue = NewObject<USCJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

void USCJsonObject::SetArrayField(const FString& FieldName, const TArray<USCJsonValue*>& InArray)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	TArray< TSharedPtr<FJsonValue> > ValArray;

	// Process input array and COPY original values
	for (auto InVal : InArray)
	{
		TSharedPtr<FJsonValue> JsonVal = InVal->GetRootValue();

		switch (InVal->GetType())
		{
		case ESCJson::None:
			break;

		case ESCJson::Null:
			ValArray.Add(MakeShareable(new FJsonValueNull()));
			break;

		case ESCJson::String:
			ValArray.Add(MakeShareable(new FJsonValueString(JsonVal->AsString())));
			break;

		case ESCJson::Number:
			ValArray.Add(MakeShareable(new FJsonValueNumber(JsonVal->AsNumber())));
			break;

		case ESCJson::Boolean:
			ValArray.Add(MakeShareable(new FJsonValueBoolean(JsonVal->AsBool())));
			break;

		case ESCJson::Array:
			ValArray.Add(MakeShareable(new FJsonValueArray(JsonVal->AsArray())));
			break;

		case ESCJson::Object:
			ValArray.Add(MakeShareable(new FJsonValueObject(JsonVal->AsObject())));
			break;

		default:
			break;
		}
	}

	JsonObj->SetArrayField(FieldName, ValArray);
}

void USCJsonObject::MergeJsonObject(USCJsonObject* InJsonObject, bool Overwrite)
{
	TArray<FString> Keys = InJsonObject->GetFieldNames();
	
	for (auto Key : Keys)
	{
		if (Overwrite == false && HasField(Key))
		{
			continue;
		}
		
		SetField(Key, InJsonObject->GetField(Key));
	}
}

USCJsonObject* USCJsonObject::GetObjectField(const FString& FieldName) const
{
	if (!JsonObj.IsValid() || !JsonObj->HasTypedField<EJson::Object>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Object"), *FieldName);
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObjField = JsonObj->GetObjectField(FieldName);

	USCJsonObject* OutRestJsonObj = NewObject<USCJsonObject>();
	OutRestJsonObj->SetRootObject(JsonObjField);

	return OutRestJsonObj;
}

void USCJsonObject::SetObjectField(const FString& FieldName, USCJsonObject* JsonObject)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetObjectField(FieldName, JsonObject->GetRootObject());
}


TArray<uint8> USCJsonObject::GetBinaryField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::String>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type String"), *FieldName);
	}
	TSharedPtr<FJsonValue> JsonValue = JsonObj->TryGetField(FieldName);

	if (FJsonValueBinary::IsBinary(JsonValue))
	{
		return FJsonValueBinary::AsBinary(JsonValue);
	}
	else
	{
		TArray<uint8> EmptyArray;
		return EmptyArray;
	}
}

void USCJsonObject::SetBinaryField(const FString& FieldName, const TArray<uint8>& Bytes)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}
	TSharedPtr<FJsonValueBinary> JsonValue = MakeShareable(new FJsonValueBinary(Bytes));
	JsonObj->SetField(FieldName, JsonValue);
}

//////////////////////////////////////////////////////////////////////////
// Array fields helpers (uniform arrays)

TArray<float> USCJsonObject::GetNumberArrayField(const FString& FieldName)
{
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Array"), *FieldName);
	}

	TArray<float> NumberArray;
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return NumberArray;
	}

	TArray<TSharedPtr<FJsonValue> > JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue> >::TConstIterator It(JsonArrayValues); It; ++It)
	{
		auto Value = (*It).Get();
		if (Value->Type != EJson::Number)
		{
			UE_LOG(LogSCJson, Error, TEXT("Not Number element in array with field name %s"), *FieldName);
		}
		
		NumberArray.Add((*It)->AsNumber());
	}

	return NumberArray;
}

void USCJsonObject::SetNumberArrayField(const FString& FieldName, const TArray<float>& NumberArray)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	TArray< TSharedPtr<FJsonValue> > EntriesArray;

	for (auto Number : NumberArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueNumber(Number)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

TArray<FString> USCJsonObject::GetStringArrayField(const FString& FieldName)
{
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Array"), *FieldName);
	}

	TArray<FString> StringArray;
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return StringArray;
	}

	TArray<TSharedPtr<FJsonValue> > JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue> >::TConstIterator It(JsonArrayValues); It; ++It)
	{
		auto Value = (*It).Get();
		if (Value->Type != EJson::String)
		{
			UE_LOG(LogSCJson, Error, TEXT("Not String element in array with field name %s"), *FieldName);
		}

		StringArray.Add((*It)->AsString());
	}

	return StringArray;
}

void USCJsonObject::SetStringArrayField(const FString& FieldName, const TArray<FString>& StringArray)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	TArray< TSharedPtr<FJsonValue> > EntriesArray;
	for (auto String : StringArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueString(String)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

TArray<bool> USCJsonObject::GetBoolArrayField(const FString& FieldName)
{
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Array"), *FieldName);
	}

	TArray<bool> BoolArray;
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return BoolArray;
	}

	TArray<TSharedPtr<FJsonValue> > JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue> >::TConstIterator It(JsonArrayValues); It; ++It)
	{
		auto Value = (*It).Get();
		if (Value->Type != EJson::Boolean)
		{
			UE_LOG(LogSCJson, Error, TEXT("Not Boolean element in array with field name %s"), *FieldName);
		}

		BoolArray.Add((*It)->AsBool());
	}

	return BoolArray;
}

void USCJsonObject::SetBoolArrayField(const FString& FieldName, const TArray<bool>& BoolArray)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	TArray< TSharedPtr<FJsonValue> > EntriesArray;
	for (auto Boolean : BoolArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueBoolean(Boolean)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

TArray<USCJsonObject*> USCJsonObject::GetObjectArrayField(const FString& FieldName)
{
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogSCJson, Warning, TEXT("No field with name %s of type Array"), *FieldName);
	}

	TArray<USCJsonObject*> OutArray;
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return OutArray;
	}

	TArray< TSharedPtr<FJsonValue> > ValArray = JsonObj->GetArrayField(FieldName);
	for (auto Value : ValArray)
	{
		if (Value->Type != EJson::Object)
		{
			UE_LOG(LogSCJson, Error, TEXT("Not Object element in array with field name %s"), *FieldName);
		}

		TSharedPtr<FJsonObject> NewObj = Value->AsObject();

		USCJsonObject* NewJson = NewObject<USCJsonObject>();
		NewJson->SetRootObject(NewObj);

		OutArray.Add(NewJson);
	}

	return OutArray;
}

void USCJsonObject::SetObjectArrayField(const FString& FieldName, const TArray<USCJsonObject*>& ObjectArray)
{
	if (!JsonObj.IsValid() || FieldName.IsEmpty())
	{
		return;
	}

	TArray< TSharedPtr<FJsonValue> > EntriesArray;
	for (auto Value : ObjectArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueObject(Value->GetRootObject())));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}
