// Modifications Copyright 2020-current ZiiCreater LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved


// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SCJsonConvert.h"
#include "SCJsonObject.h"
#include "SCJsonLibrary.generated.h"

/**
 * Usefull tools for REST communications
 */
UCLASS()
class SCJSON_API USCJsonLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	//////////////////////////////////////////////////////////////////////////
	// Helpers

public:
	/** Applies percent-encoding to text */
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Utility")
	static FString PercentEncode(const FString& Source);

	/**
	 * Encodes a FString into a Base64 string
	 *
	 * @param Source	The string data to convert
	 * @return			A string that encodes the binary data in a way that can be safely transmitted via various Internet protocols
	 */
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Utility", meta = (DisplayName = "Base64 Encode (String)"))
	static FString Base64Encode(const FString& Source);

	/**
	 * Encodes a Byte array into a Base64 string
	 *
	 * @param Source	The string data to convert
	 * @return			A string that encodes the binary data in a way that can be safely transmitted via various Internet protocols
	 */
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Utility", meta = (DisplayName = "Base64 Encode (Bytes)"))
	static FString Base64EncodeBytes(const TArray<uint8>& Source);

	/**
	 * Decodes a Base64 string into a FString
	 *
	 * @param Source	The stringified data to convert
	 * @param Dest		The out buffer that will be filled with the decoded data
	 * @return			True if the buffer was decoded, false if it failed to decode
	 */
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Utility", meta = (DisplayName = "Base64 Decode (To String)"))
	static bool Base64Decode(const FString& Source, FString& Dest);


	/**
	 * Decodes a Base64 string into a Byte array
	 *
	 * @param Source	The stringified data to convert
	 * @param Dest		The out buffer that will be filled with the decoded data
	 * @return			True if the buffer was decoded, false if it failed to decode
	 */
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Utility", meta = (DisplayName = "Base64 Decode (To Bytes)"))
	static bool Base64DecodeBytes(const FString& Source, TArray<uint8>& Dest);

	//////////////////////////////////////////////////////////////////////////
	// Easy URL processing

	/**
	* Decodes a json string into an array of stringified json Values
	*
	* @param JsonString				Input stringified json
	* @param OutJsonValueArray		The decoded Array of JsonValue 
	*/
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Utility")
	static bool StringToJsonValueArray(const FString& JsonString, TArray<USCJsonValue*>& OutJsonValueArray);

	/**
	* Uses the reflection system to convert an unreal struct into a JsonObject
	*
	* @param AnyStruct		The struct you wish to convert
	* @return				Converted Json Object
	*/
	UFUNCTION(BlueprintPure, Category = "SocketCluster|Utility", CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
	static USCJsonObject* StructToJsonObject(TFieldPath<FProperty> AnyStruct);

	/**
	* Uses the reflection system to fill an unreal struct from data defined in JsonObject.
	*
	* @param JsonObject		The source JsonObject for properties to fill
	* @param AnyStruct		The struct you wish to fill
	* @return				Whether all properties filled correctly
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Utility", CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
	static bool JsonObjectToStruct(USCJsonObject* JsonObject, TFieldPath<FProperty> AnyStruct);

	//Convert property into c++ accessible form
	DECLARE_FUNCTION(execStructToJsonObject)
	{
		//Get properties and pointers from stack
		Stack.Step(Stack.Object, NULL);
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;

		// We need this to wrap up the stack
		P_FINISH;

		auto BPJsonObject = NewObject<USCJsonObject>();

		auto JsonObject = USCJsonConvert::ToJsonObject(StructProperty->Struct, StructPtr, true);
		BPJsonObject->SetRootObject(JsonObject);

		*(USCJsonObject**)RESULT_PARAM = BPJsonObject;
	}

	DECLARE_FUNCTION(execJsonObjectToStruct)
	{
		//Get properties and pointers from stack
		P_GET_OBJECT(USCJsonObject, JsonObject);

		Stack.Step(Stack.Object, NULL);
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;

		//Pass in the reference to the json object
		bool Success = USCJsonConvert::JsonObjectToUStruct(JsonObject->GetRootObject(), StructProperty->Struct, StructPtr, true);

		*(bool*)RESULT_PARAM = Success;
	}

	//Convenience - Saving/Loading structs from files
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Utility", CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
	static bool SaveStructToJsonFile(TFieldPath<FProperty> AnyStruct, const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Utility", CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
	static bool LoadJsonFileToStruct(const FString& FilePath, TFieldPath<FProperty> AnyStruct);

	//custom thunk needed to handle wildcard structs
	DECLARE_FUNCTION(execSaveStructToJsonFile)
	{
		Stack.StepCompiledIn<FStructProperty>(NULL);
		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;

		FString FilePath;
		Stack.StepCompiledIn<FStrProperty>(&FilePath);
		P_FINISH;

		P_NATIVE_BEGIN;
		*(bool*)RESULT_PARAM = USCJsonConvert::ToJsonFile(FilePath, StructProp->Struct, StructPtr);
		P_NATIVE_END;
	}

	//custom thunk needed to handle wildcard structs
	DECLARE_FUNCTION(execLoadJsonFileToStruct)
	{
		FString FilePath;
		Stack.StepCompiledIn<FStrProperty>(&FilePath);
		Stack.StepCompiledIn<FStructProperty>(NULL);
		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;
		P_FINISH;

		P_NATIVE_BEGIN;
		*(bool*)RESULT_PARAM = USCJsonConvert::JsonFileToUStruct(FilePath, StructProp->Struct, StructPtr, true);
		P_NATIVE_END;
	}

	//ConverSCn Nodes

	//ToJsonValue

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (Array)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_ArrayToJsonValue(const TArray<USCJsonValue*>& InArray);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (JsonObject)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_JsonObjectToJsonValue(USCJsonObject* InObject);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (Bytes)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_BytesToJsonValue(const TArray<uint8>& InBytes);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (String)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_StringToJsonValue(const FString& InString);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (Integer)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_IntToJsonValue(int32 InInt);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (Float)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_FloatToJsonValue(float InFloat);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To JsonValue (Bool)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonValue* Conv_BoolToJsonValue(bool InBool);

	//To Native Types
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Integer (JsonValue)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static int32 Conv_JsonValueToInt(class USCJsonValue* InValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Float (JsonValue)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static float Conv_JsonValueToFloat(class USCJsonValue* InValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Bool (JsonValue)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static bool Conv_JsonValueToBool(class USCJsonValue* InValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Bytes (JsonValue)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static TArray<uint8> Conv_JsonValueToBytes(class USCJsonValue* InValue);

	//ToString - these never get called sadly, kismet library get display name takes priority
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To String (SCJsonObject)", CompactNodeTitle = "->", BlueprintAutocast), Category = "SocketCluster|Utility")
	static FString Conv_JsonObjectToString(class USCJsonObject* InObject);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Object (JsonValue)", BlueprintAutocast), Category = "SocketCluster|Utility")
	static USCJsonObject* Conv_JsonValueToJsonObject(class USCJsonValue* InValue);

};
