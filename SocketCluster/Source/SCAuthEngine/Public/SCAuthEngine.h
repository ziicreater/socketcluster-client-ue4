// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCAuthEngine.generated.h"

class FJsonValue;

/**
* The SocketCluster AuthEngine
*/
UCLASS(BlueprintType, Abstract)
class SCAUTHENGINE_API USCAuthEngine : public UObject
{
	GENERATED_BODY()

public:

	virtual void saveToken(FString name, FString token, TFunction<void(TSharedPtr<FJsonValue>, FString)> callback);

	virtual void removeToken(FString name, TFunction<void(TSharedPtr<FJsonValue>, FString)> callback);

	virtual void loadToken(FString name, TFunction<void(TSharedPtr<FJsonValue>, FString)> callback);

};

