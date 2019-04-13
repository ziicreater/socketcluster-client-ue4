// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/DelegateCombinations.h"
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

	USCJsonObject* data;

	TFunction<void(USCJsonObject* error, USCJsonObject* data)> callback;

	FTimerDelegate timeout;

	FTimerHandle timeoutHandle;

};
