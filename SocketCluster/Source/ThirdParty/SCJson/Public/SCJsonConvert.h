// Modifications Copyright 2020-current ZiiCreater LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved


#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "UObject/ObjectMacros.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "Runtime/Json/Public/Dom/JsonValue.h"
#include "SCJsonConvert.generated.h"

struct FTrimmedKeyMap
{
	FString LongKey;
	TMap<FString, TSharedPtr<FTrimmedKeyMap>> SubMap;

	FString ToString();
};

/**
 * 
 */
UCLASS()
class SCJSON_API USCJsonConvert : public UObject
{
	GENERATED_BODY()
public:

	//encode/decode json convenience wrappers
	static FString ToJsonString(const TSharedPtr<FJsonObject>& JsonObject);
	static FString ToJsonString(const TSharedPtr<FJsonValue>& JsonValue);
	static FString ToJsonString(const TArray<TSharedPtr<FJsonValue>>& JsonValueArray);

	static TSharedPtr<FJsonObject> ToJsonObject(const FString& JsonString);
	static TSharedPtr<FJsonObject> MakeJsonObject();


	//Structs

	//Will trim names if specified as blueprint
	static TSharedPtr<FJsonObject> ToJsonObject(UStruct* Struct, void* StructPtr, bool IsBlueprintStruct = false, bool BinaryStructCppSupport = false);

	//Expects a JsonObject, if blueprint struct it will lengthen the names to fill properly
	static bool JsonObjectToUStruct(TSharedPtr<FJsonObject> JsonObject, UStruct* Struct, void* StructPtr, bool IsBlueprintStruct = false, bool BinaryStructCppSupport = false);
	
	//Files - convenience read/write files
	static bool JsonFileToUStruct(const FString& FilePath, UStruct* Struct, void* StructPtr, bool IsBlueprintStruct = false);
	static bool ToJsonFile(const FString& FilePath, UStruct* Struct, void* StructPtr, bool IsBlueprintStruct = false);
		

	//typically from callbacks
	static class USCJsonValue* ToSCJsonValue(const TArray<TSharedPtr<FJsonValue>>& JsonValueArray);

	//Convenience overrides for JsonValues
	static TSharedPtr<FJsonValue> ToJsonValue(const TSharedPtr<FJsonObject>& JsonObject);
	static TSharedPtr<FJsonValue> ToJsonValue(const FString& StringValue);
	static TSharedPtr<FJsonValue> ToJsonValue(double NumberValue);
	static TSharedPtr<FJsonValue> ToJsonValue(bool BoolValue);
	static TSharedPtr<FJsonValue> ToJsonValue(const TArray<uint8>& BinaryValue);
	static TSharedPtr<FJsonValue> ToJsonValue(const TArray<TSharedPtr<FJsonValue>>& ArrayValue);

	static TSharedPtr<FJsonValue> JsonStringToJsonValue(const FString& JsonString);
	static TArray<TSharedPtr<FJsonValue>> JsonStringToJsonArray(const FString& JsonString);


	//internal utility, exposed for modularity
	static void TrimValueKeyNames(const TSharedPtr<FJsonValue>& JsonValue);
	static bool TrimKey(const FString& InLongKey, FString& OutTrimmedKey);
	static void SetTrimmedKeyMapForStruct(TSharedPtr<FTrimmedKeyMap>& InMap, UStruct* Struct);
	static void SetTrimmedKeyMapForProp(TSharedPtr<FTrimmedKeyMap>& InMap, FProperty* ArrayInnerProp);
	static void ReplaceJsonValueNamesWithMap(TSharedPtr<FJsonValue>& InValue, TSharedPtr<FTrimmedKeyMap> KeyMap);

	static FString EnumToString(const FString& enumName, const int32 value);

	static TSharedPtr<FJsonObject> Merge(const TSharedPtr<FJsonObject>& Target, const TSharedPtr<FJsonObject>& Sources);
}; 


