// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SocketClusterJsonObject.h"
#include "SocketClusterCodecEngine.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USocketClusterCodecEngine : public UInterface
{
	GENERATED_BODY()
};

/**
*
*/
class SOCKETCLUSTERCODECENGINE_API ISocketClusterCodecEngine
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual FString Encode(USocketClusterJsonObject* input);

	virtual USocketClusterJsonObject* Decode(FString input);

};
