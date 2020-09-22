// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCClientSocket.h"
#include "Json.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCClient.generated.h"

/**
 * SocketCluster Client
 */
UCLASS()
class SCCLIENT_API USCClient : public UObject
{
	GENERATED_BODY()

private:

	/* Checks if the hostname provided starts with HTTPS */
	static bool IsUrlSecure(FString Hostname);

	/* Get the correct port compare to the hostname */
	static int32 GetPort(int32 Port, bool Secure, bool IsSecureDefault);

public:

	/**
	* Creates and returns a new socket connection to the specified host. The options argument is optional
	*/
	static USCClientSocket* Create(TSharedPtr<FJsonObject> Options, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine);
	
};
