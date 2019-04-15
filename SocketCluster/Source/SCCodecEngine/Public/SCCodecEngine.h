// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonObject.h"
#include "SCCodecEngine.generated.h"

/**
* The SocketCluster CodecEngine
*/
UCLASS(Abstract)
class SCCODECENGINE_API USCCodecEngine : public UObject
{
	GENERATED_BODY()

public:
	
	virtual FString encode(USCJsonObject* Object);

	virtual USCJsonObject* decode(const FString& Input);

};