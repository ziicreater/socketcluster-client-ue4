// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "SCJsonObject.generated.h"

class USCJsonValue;
class FJsonObject;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SCJSON_API USCJsonObject : public UObject
{
	GENERATED_BODY()

public:

	USCJsonObject();

	/** Create new Json object */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Object", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "SocketCluster|Json")
		static USCJsonObject* ConstructJsonObject(UObject* WorldContextObject);

	/** Reset all internal data */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void Reset();

	/** Get the root Json object */
	TSharedPtr<FJsonObject>& GetRootObject();

	/** Set the root Json object */
	void SetRootObject(TSharedPtr<FJsonObject>& JsonObject);

	/** Serialize Json to string (formatted with line breaks) */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		FString EncodeJson() const;

	/** Serialize Json to string (single string without line breaks) */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		FString EncodeJsonToSingleString() const;

	/** Construct Json object from string */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		bool DecodeJson(const FString& JsonString);

	/** Returns a list of field names that exist in the object */
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Json")
		TArray<FString> GetFieldNames() const;

	/** Checks to see if the FieldName exists in the object */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		bool HasField(const FString& FieldName) const;

	/** Remove field named FieldName */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void RemoveField(const FString& FieldName);

	/** Get the field named FieldName as a JsonValue */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		USCJsonValue* GetField(const FString& FieldName) const;

	/** Add a field named FieldName with a Value */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetField(const FString& FieldName, USCJsonValue* JsonValue);

	/** Get the field named FieldName as a Json Array */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		TArray<USCJsonValue*> GetArrayField(const FString& FieldName) const;

	/** Set an ObjectField named FieldName and value of Json Array */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetArrayField(const FString& FieldName, const TArray<USCJsonValue*>& InArray);

	/** Adds all of the fields from one json object to this one */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void MergeJsonObject(USCJsonObject* InJsonObject, bool Overwrite);

	/** Get the field named FieldName as a number. Ensures that the field is present and is of type Json number.
	* Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		float GetNumberField(const FString& FieldName) const;

	/** Add a field named FieldName with Number as value
	* Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetNumberField(const FString& FieldName, float Number);

	/** Get the field named FieldName as an Integer. Ensures that the field is present and is of type Json number. */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		int32 GetIntegerField(const FString& FieldName) const;

	/** Add a field named FieldName with Integer as value. */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetIntegerField(const FString& FieldName, int32 Number);

	/** Get the field named FieldName as a string. */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		FString GetStringField(const FString& FieldName) const;

	/** Add a field named FieldName with value of StringValue */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetStringField(const FString& FieldName, const FString& StringValue);

	/** Get the field named FieldName as a boolean. */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		bool GetBoolField(const FString& FieldName) const;

	/** Set a boolean field named FieldName and value of InValue */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetBoolField(const FString& FieldName, bool InValue);

	/** Get the field named FieldName as a Json object. */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		USCJsonObject* GetObjectField(const FString& FieldName) const;

	/** Set an ObjectField named FieldName and value of JsonObject */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetObjectField(const FString& FieldName, USCJsonObject* JsonObject);

	/** Get the field named FieldName as a Number Array. Use it only if you're sure that array is uniform!
	* Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		TArray<float> GetNumberArrayField(const FString& FieldName) const;

	/** Set an ObjectField named FieldName and value of Number Array
	* Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetNumberArrayField(const FString& FieldName, const TArray<float>& NumberArray);

	/** Get the field named FieldName as a String Array. Use it only if you're sure that array is uniform! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		TArray<FString> GetStringArrayField(const FString& FieldName) const;

	/** Set an ObjectField named FieldName and value of String Array */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetStringArrayField(const FString& FieldName, const TArray<FString>& StringArray);

	/** Get the field named FieldName as a Bool Array. Use it only if you're sure that array is uniform! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		TArray<bool> GetBoolArrayField(const FString& FieldName) const;

	/** Set an ObjectField named FieldName and value of Bool Array */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetBoolArrayField(const FString& FieldName, const TArray<bool>& BoolArray);

	/** Get the field named FieldName as an Object Array. Use it only if you're sure that array is uniform! */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		TArray<USCJsonObject*> GetObjectArrayField(const FString& FieldName) const;

	/** Set an ObjectField named FieldName and value of Ob Array */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Json")
		void SetObjectArrayField(const FString& FieldName, const TArray<USCJsonObject*>& ObjectArray);

	/** Deserialize byte content to json */
	int32 DeserializeFromUTF8Bytes(const ANSICHAR* Bytes, int32 Size);

	/** Deserialize byte content to json */
	int32 DeserializeFromTCHARBytes(const TCHAR* Bytes, int32 Size);

	/** Deserialize byte stream from reader */
	void DecodeFromArchive(TUniquePtr<FArchive>& Reader);

	/** Save json to file */
	bool WriteToFile(const FString& Path);

	static bool WriteStringToArchive(FArchive& Ar, const TCHAR* StrPtr, int64 Len);

private:

	/** Internal JSON data */
	TSharedPtr<FJsonObject> JsonObj;
	
	
};
