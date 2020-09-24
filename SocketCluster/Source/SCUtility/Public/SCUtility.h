// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCUtility.generated.h"

UENUM(BlueprintType, DisplayName = "SCSocketState")
enum class ESCSocketState : uint8
{
	CLOSED,
	CONNECTING,
	OPEN
};

UENUM(BlueprintType, DisplayName = "SCAuthState")
enum class ESCAuthState : uint8
{
	AUTHENTICATED,
	UNAUTHENTICATED
};

/**
 * SocketCluster Utility
 */
UCLASS()
class SCUTILITY_API USCUtility : public UObject
{
	GENERATED_BODY()
	
};
