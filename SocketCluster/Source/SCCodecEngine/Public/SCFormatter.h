// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCCodecEngine.h"
#include "SCJsonConvert.h"
#include "SCFormatter.generated.h"

/**
 * SocketCluster Default CodecEngine
 * 
 * @Encode
 * Converts Json Object To String
 * 
 * @Decode
 * Converts String to Json Object
 * 
 */
UCLASS()
class SCCODECENGINE_API USCFormatter : public USCCodecEngine
{
	GENERATED_BODY()

public:

	/**
	* Encode Data
	*
	* @param Data		Data to Encode
	*
	* @return			Returns encoded data in FString format.
	*/
	virtual FString Encode(TSharedPtr<FJsonValue> Data) override;

	/**
	* Decode Data
	*
	* @param Data		Data to Decode
	*
	* @return			Returns Decoded data in JsonValue.
	*/
	virtual TSharedPtr<FJsonValue> Decode(FString Data) override;
	
};
