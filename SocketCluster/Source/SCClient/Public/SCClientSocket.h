// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCCodecEngine.h"
#include "SCAuthEngine.h"
#include "SCTransport.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "SCClientSocket.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SCCLIENT_API USCClientSocket : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	USCCodecEngine* codec;

	UPROPERTY()
	USCAuthEngine* auth;

	UPROPERTY()
	USCTransport* transport;

public:

	void Create(TSharedPtr<FJsonObject> Options);

};
