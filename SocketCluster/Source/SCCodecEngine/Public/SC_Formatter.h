// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCCodecEngine.h"
#include "SC_Formatter.generated.h"

/**
* The SocketCluster Formatter
*/
UCLASS()
class SCCODECENGINE_API USC_Formatter : public USCCodecEngine
{
	GENERATED_BODY()

public:

	//FString arrayBufferToBase64();

	//FString binaryToBase64Replacer();

	virtual FString encode(TSharedPtr<FJsonValue> object) override;

	virtual TSharedPtr<FJsonValue> decode(const FString& input) override;

};
