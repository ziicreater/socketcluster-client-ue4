// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SCClient.h"
#include "SCClientSocket.h"
#include "SCCodecEngine.h"
#include "SCAuthEngine.h"
#include "SCJsonObject.h"
#include "SCClientBlueprint.generated.h"

/**
 * SocketCluster Blueprint Client
 */
UCLASS()
class SCCLIENT_API USCClientBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	* 
	*/
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Query", AdvancedDisplay = "Query, AuthEngine, CodecEngine, ProtocolVersion, AckTimeOut, AutoConnect, AutoReconnect, ReconnectInitialDelay, ReconnectRandomness, ReconnectMultiplier, ReconnectMaxDelay, PubSubBatchDuration, ConnectTimeout, PingTimeoutDisabled, TimestampRequests, TimestampParam, AuthTokenName, RejectUnauthorized, AutoSubscribeOnConnect, ChannelPrefix"), Category = "SocketCluster|Client")
	static USCClientSocket* Create(
		USCJsonObject* Query,
		TSubclassOf<USCAuthEngine> AuthEngine,
		TSubclassOf<USCCodecEngine> CodecEngine,
		const FString& Hostname = FString(TEXT("localhost")),
		const bool Secure = false,
		const int32 Port = 80,
		const FString& Path = FString(TEXT("/socketcluster/")),
		const int32 ProtocolVersion = 2,
		const float AckTimeOut = 10.0f,
		const bool AutoConnect = true,
		const bool AutoReconnect = true,
		const float ReconnectInitialDelay = 10.0f,
		const float ReconnectRandomness = 10.0f,
		const float ReconnectMultiplier = 1.5,
		const float ReconnectMaxDelay = 60.0f,
		const float PubSubBatchDuration = 0.0f,
		const float ConnectTimeout = 0.0f,
		const bool PingTimeoutDisabled = false,
		const bool TimestampRequests = false,
		const FString& TimestampParam = FString(TEXT("t")),
		const FString& AuthTokenName = FString(TEXT("socketCluster.authToken")),
		const bool Multiplex = true,
		const bool RejectUnauthorized = true,
		const bool AutoSubscribeOnConnect = true,
		const FString& ChannelPrefix = FString(TEXT(""))
	);


};
