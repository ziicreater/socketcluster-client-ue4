// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "SocketClusterJsonValue.generated.h"

class USocketClusterJsonObject;
class FJsonValue;

/**
* Represents all the types a Json Value can be.
*/
UENUM(BlueprintType)
namespace EVaJson
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
	};
}

/**
* Blueprintable FJsonValue wrapper
*/
UCLASS(BlueprintType, Blueprintable)
class SOCKETCLUSTERJSON_API USocketClusterJsonValue : public UObject
{
	GENERATED_BODY()

public:

	/** Create new Json Number value
	* Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Number Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
		static USocketClusterJsonValue* ConstructJsonValueNumber(UObject* WorldContextObject, float Number);

	/** Create new Json String value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json String Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
		static USocketClusterJsonValue* ConstructJsonValueString(UObject* WorldContextObject, const FString& StringValue);

	/** Create new Json Bool value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Bool Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
		static USocketClusterJsonValue* ConstructJsonValueBool(UObject* WorldContextObject, bool InValue);

	/** Create new Json Array value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Array Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
		static USocketClusterJsonValue* ConstructJsonValueArray(UObject* WorldContextObject, const TArray<USocketClusterJsonValue*>& InArray);

	/** Create new Json Object value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Object Value", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
		static USocketClusterJsonValue* ConstructJsonValueObject(UObject* WorldContextObject, USocketClusterJsonObject* JsonObject);

	/** Create new Json value from FJsonValue (to be used from SocketClusterJsonObject) */
	static USocketClusterJsonValue* ConstructJsonValue(UObject* WorldContextObject, const TSharedPtr<FJsonValue>& InValue);

	/** Get the root Json value */
	TSharedPtr<FJsonValue>& GetRootValue();

	/** Set the root Json value */
	void SetRootValue(TSharedPtr<FJsonValue>& JsonValue);

	/** Get type of Json value (Enum) */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		EVaJson::Type GetType() const;

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
		TArray<USocketClusterJsonValue*> AsArray() const;

	/** Returns this value as an object, throwing an error if this is not an Json Object */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		USocketClusterJsonObject* AsObject();

private:

	/** Internal JSON data */
	TSharedPtr<FJsonValue> JsonVal;

protected:

	/** Simple error logger */
	void ErrorMessage(const FString& InType) const;

};

