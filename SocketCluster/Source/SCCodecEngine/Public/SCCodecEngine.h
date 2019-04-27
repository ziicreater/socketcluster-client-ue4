// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonConvert.h"
#include "SCJsonValue.h"
#include "SCCodecEngine.generated.h"

/**
* The SocketCluster CodecEngine
*/
UCLASS(Abstract)
class SCCODECENGINE_API USCCodecEngine : public UObject
{
	GENERATED_BODY()

public:
	
	virtual FString encode(TSharedPtr<FJsonValue> Object);

	virtual TSharedPtr<FJsonValue> decode(const FString& Input);

};