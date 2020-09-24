// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCSocket.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCUtility.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Containers/Ticker.h"
#include "SCTransport.generated.h"

/**
 * 
 */
UCLASS()
class SCCLIENT_API USCTransport : public UObject
{
	GENERATED_BODY()

	virtual class UWorld* GetWorld() const override;

	UPROPERTY()
	USCSocket* Socket;

	ESCSocketState State;

	FTimerDelegate _ConnectTimeoutRef;
	FTimerHandle _ConnectTimeoutHandle;
	
	float ConnectTimeOut;

public:

	//TFunction<>

	void Create(TSharedPtr<FJsonObject> Options);

	//void Create(TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine, TSharedPtr<FJsonObject> Options);
	
};
