// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCAuthEngine.generated.h"

/**
 * SocketCluster AuthEngine Interface
 */
UCLASS()
class SCAUTHENGINE_API USCAuthEngine : public UObject
{
	GENERATED_BODY()

public:
	
	virtual FString saveToken(FString name, FString token);

	virtual FString removeToken(FString name);

	virtual FString loadToken(FString name);

};
