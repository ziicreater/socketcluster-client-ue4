// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SCSocket.h"
#include "SCClient.generated.h"

/**
 * 
 */
UCLASS()
class SCCLIENT_API USCClient : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Test"), Category = "SocketCluster|Client")
	static USCSocket* Test();
	
};
