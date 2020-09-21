// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCAuthEngine.h"
#include "SCDefaultAuthEngine.generated.h"

/**
 * 
 */
UCLASS()
class SCAUTHENGINE_API USCDefaultAuthEngine : public USCAuthEngine
{

	GENERATED_BODY()

protected:

	TMap<FString, FString> localStorage;

public:

	virtual FString saveToken(FString name, FString token) override;

	virtual FString removeToken(FString name) override;

	virtual FString loadToken(FString name) override;
};
