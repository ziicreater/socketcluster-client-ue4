// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCJsonConvert.h"
#include "SCCodecEngine.generated.h"

/**
 * SocketCluster CodecEngine Interface
 */
UCLASS(Abstract)
class SCCODECENGINE_API USCCodecEngine : public UObject
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
	virtual FString Encode(TSharedPtr<FJsonValue> Data);

	/**
	* Decode Data
	*
	* @param Data		Data to Decode
	*
	* @return			Returns Decoded data in JsonValue format.
	*/
	virtual TSharedPtr<FJsonValue> Decode(FString Data);
	
};
