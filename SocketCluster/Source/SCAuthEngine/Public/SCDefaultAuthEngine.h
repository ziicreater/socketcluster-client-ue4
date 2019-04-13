// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCAuthEngine.h"
#include "SCJsonObject.h"
#include "SCDefaultAuthEngine.generated.h"

/**
* The SocketCluster DefaultAuthEngine
*/
UCLASS()
class SCAUTHENGINE_API USCDefaultAuthEngine : public USCAuthEngine
{
	GENERATED_BODY()
	
protected:

	TMap<FString, FString> Storage;

public:

	virtual void saveToken(FString name, FString token, TFunction<void(USCJsonObject*, FString&)> callback) override;

	virtual void removeToken(FString name, TFunction<void(USCJsonObject*, FString&)> callback) override;

	virtual void loadToken(FString name, TFunction<void(USCJsonObject*, FString&)> callback) override;
};
