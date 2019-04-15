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

	virtual FString encode(USCJsonObject* object) override;

	virtual USCJsonObject* decode(const FString& input) override;

};
