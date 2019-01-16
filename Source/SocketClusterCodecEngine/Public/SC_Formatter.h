// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SocketClusterCodecEngine.h"
#include "SocketClusterJsonObject.h"
#include "SC_Formatter.generated.h"

/**
 * 
 */
UCLASS()
class SOCKETCLUSTERCODECENGINE_API USC_Formatter : public UObject, public ISocketClusterCodecEngine
{
	GENERATED_BODY()
	
public:

	virtual FString Encode(USocketClusterJsonObject* input) override;

	virtual USocketClusterJsonObject* Decode(FString input) override;
	
};
