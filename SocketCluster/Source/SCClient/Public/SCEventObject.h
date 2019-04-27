// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "SCJsonObject.h"
#include "SCEventObject.generated.h"

/**
* The SocketCluster EventObject
*/
UCLASS()
class SCCLIENT_API USCEventObject : public UObject
{
	GENERATED_BODY()

	USCEventObject();

public:

	int32 cid;

	FString event;

	TSharedPtr<FJsonValue> data;

	TFunction<void(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)> callback;

	FTimerDelegate timeout;

	FTimerHandle timeoutHandle;

};
