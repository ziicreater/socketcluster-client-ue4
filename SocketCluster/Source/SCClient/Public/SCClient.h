// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SCClientSocket.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCJsonObject.h"
#include "SCClient.generated.h"

/**
* The SocketCluster Client
*/
UCLASS()
class SCCLIENT_API USCClient : public UBlueprintFunctionLibrary
{

	GENERATED_BODY()

private:

	static FString GetMultiplexId(const USCJsonObject* options);

	static bool isUrlSecure(const FString& hostname);

	static int32 GetPort(const int32 port, const bool secure, const bool isSecureDefault);

public:

	static TMap<FString, USCClientSocket*> _clients;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clients"), Category = "SocketCluster|Client")
		static TMap<FString, USCClientSocket*> Clients();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Version"), Category = "SocketCluster|Client")
		static FString Version();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy"), Category = "SocketCluster|Client")
		static void Destroy(USCClientSocket* Socket);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Query", 
		AdvancedDisplay = "Query, CodecEngine, AuthEngine, AckTimeOut, AutoConnect, AutoReconnect, ReconnectInitialDelay, ReconnectRandomness, ReconnectMultiplier, ReconnectMaxDelay, PubSubBatchDuration, ConnectTimeout, PingTimeoutDisabled, TimestampRequests, TimestampParam, AuthTokenName, RejectUnauthorized, CloneData, AutoSubscribeOnConnect, ChannelPrefix"), Category = "SocketCluster|Client")
		static USCClientSocket* Create(
			const UObject* WorldContextObject,
			TArray<FSocketClusterKeyValue> Query,
			TSubclassOf<USCAuthEngine> AuthEngine,
			TSubclassOf<USCCodecEngine> CodecEngine,
			const FString& Hostname = FString(TEXT("localhost")),
			const bool Secure = false,
			const int32 Port = 80,
			const FString& Path = FString(TEXT("/socketcluster/")),
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
			const bool CloneData = false,
			const bool AutoSubscribeOnConnect = true,
			const FString& ChannelPrefix = FString(TEXT(""))
		);
};

