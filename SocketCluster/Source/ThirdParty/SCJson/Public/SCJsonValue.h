// Modifications Copyright 2019 ZiiCreater, LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved
// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Runtime/Json/Public/Dom/JsonValue.h"
#include "SCJsonValue.generated.h"

class USCJsonObject;

/**
 * Represents all the types a Json Value can be.
 */
UENUM(BlueprintType)
namespace ESCJson
{
	enum Type
	{
		None,
		Null,
		String,
		Number,
		Boolean,
		Array,
		Object,
		Binary
	};
}

class SCJSON_API FJsonValueBinary : public FJsonValue
{
public:
	FJsonValueBinary(const TArray<uint8>& InBinary) : Value(InBinary) { Type = EJson::String; }	//pretends to be none

	virtual bool TryGetString(FString& OutString) const override 
	{
		OutString = FString::FromHexBlob(Value.GetData(), Value.Num());	//encode the binary into the string directly
		return true;
	}
	virtual bool TryGetNumber(double& OutDouble) const override 
	{
		OutDouble = Value.Num();
		return true;
	}

	//hackery: we use this as an indicator we have a binary (strings don't normally do this)
	virtual bool TryGetBool(bool& OutBool) const override { return false; } 	

	/** Return our binary data from this value */
	TArray<uint8> AsBinary() { return Value; }

	/** Convenience method to determine if passed FJsonValue is a FJsonValueBinary or not. */
	static bool IsBinary(const TSharedPtr<FJsonValue>& InJsonValue);

	/** Convenience method to get binary array from unknown JsonValue, test with IsBinary first. */
	static TArray<uint8> AsBinary(const TSharedPtr<FJsonValue>& InJsonValue);

protected:
	TArray<uint8> Value;

	virtual FString GetType() const override { return TEXT("Binary"); }
};

/**
 * Blueprintable FJsonValue wrapper
 */
UCLASS(BlueprintType, Blueprintable)
class SCJSON_API USCJsonValue : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Create new Json Number value
	 * Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Number Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ConstructJsonValueNumber(UObject* WorldContextObject, float Number);

	/** Create new Json String value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json String Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ConstructJsonValueString(UObject* WorldContextObject, const FString& StringValue);

	/** Create new Json Bool value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Bool Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ConstructJsonValueBool(UObject* WorldContextObject, bool InValue);

	/** Create new Json Array value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Array Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ConstructJsonValueArray(UObject* WorldContextObject, const TArray<USCJsonValue*>& InArray);

	/** Create new Json Object value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Object Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ConstructJsonValueObject(USCJsonObject *JsonObject, UObject* WorldContextObject);

	/** Create new Json Binary value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Binary Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ConstructJsonValueBinary(UObject* WorldContextObject, TArray<uint8> ByteArray);

	/** Create new Json value from FJsonValue (to be used from USCJsonObject) */
	static USCJsonValue* ConstructJsonValue(UObject* WorldContextObject, const TSharedPtr<FJsonValue>& InValue);

	/** Create new Json value from JSON encoded string*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Value From Json String", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
	static USCJsonValue* ValueFromJsonString(UObject* WorldContextObject, const FString& StringValue);

	/** Get the root Json value */
	TSharedPtr<FJsonValue>& GetRootValue();

	/** Set the root Json value */
	void SetRootValue(TSharedPtr<FJsonValue>& JsonValue);


	//////////////////////////////////////////////////////////////////////////
	// FJsonValue API

	/** Get type of Json value (Enum) */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	ESCJson::Type GetType() const;

	/** Get type of Json value (String) */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	FString GetTypeString() const;

	/** Returns true if this value is a 'null' */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	bool IsNull() const;

	/** Returns this value as a double, throwing an error if this is not an Json Number
	 * Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	float AsNumber() const;

	/** Returns this value as a number, throwing an error if this is not an Json String */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	FString AsString() const;

	/** Returns this value as a boolean, throwing an error if this is not an Json Bool */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	bool AsBool() const;

	/** Returns this value as an array, throwing an error if this is not an Json Array */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	TArray<USCJsonValue*> AsArray() const;

	/** Returns this value as an object, throwing an error if this is not an Json Object */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	USCJsonObject* AsObject();

	//todo: add basic binary e.g. tarray<byte>
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	TArray<uint8> AsBinary();

	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
	FString EncodeJson() const;

	//////////////////////////////////////////////////////////////////////////
	// Data

private:
	/** Internal JSON data */
	TSharedPtr<FJsonValue> JsonVal;


	//////////////////////////////////////////////////////////////////////////
	// Helpers

protected:
	/** Simple error logger */
	void ErrorMessage(const FString& InType) const;

};
