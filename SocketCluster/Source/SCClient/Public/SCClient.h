// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCClientSocket.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "SCJsonObject.h"
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
	static bool IsUrlSecure(TSharedPtr<FJsonObject> Options);

	/* Get the correct port compare to the hostname */
	static int32 GetPort(TSharedPtr<FJsonObject> Options, bool IsSecureDefault);

public:

	/**
	* SocketCluster Client Version.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Client")
	static FString Version();

	/**
	* Creates and returns a new socket connection to the specified host.
	* 
	* @param WorldContextObject			Object with a valid UWorld.
	* @param Options
	* - 
	* -
	* 
	*/
	static USCClientSocket* Create(const UObject* WorldContextObject, TSharedPtr<FJsonObject> Options, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine);

	/**
	* Creates and returns a new socket connection to the specified host.
	* 
	* @param Query						A map of key-value pairs which will be used as query parameters for the initial HTTP handshake which will initiate the WebSocket connection.
	* @param AuthEngine					A custom engine to use for storing and loading JWT auth tokens on the client side
	* @param CodecEngine				Lets you set a custom codec engine. This allows you to specify how data gets encoded before being sent over the wire and how it gets decoded once it reaches the other side.
	* @param Hostname					Defaults to the current host.
	* @param Secure						Defaults to false.
	* @param Port						Defaults to 80 if secure is false other wise defaults to 443.
	* @param AckTimeOut					This is the timeout for getting a response to a SCSocket emit event (when a callback is provided) in milliseconds.
	* @param AutoConnect				Whether or not to automatically connect the socket as soon as it is created. Default is true.
	* @param AutoReconnect				Whether or not to automatically reconnect the socket when it loses the connection.
	* @param ReconnectInitialDelay		Initial Reconnect Delay in milliseconds.
	* @param ReconnectRandomness		Random Reconnect Delay in milliseconds.
	* @param ReconnectMultiplier		Reconnect Multiplier in decimal default is 1.5
	* @param ReconnectMaxDelay			Max Reconnect Delay in milliseconds.
	* @param PubSubBatchDuration		Defaults to null (0 milliseconds); this property affects channel subscription batching; it determines the period in milliseconds for batching multiple subscription requests together. It only affects channels that have the batch option set to true. A value of null or 0 means that all subscribe or unsubscribe requests which were made within the same call stack will be batched together
	* @param ConnectTimeout				This is the timeout for the connect event in milliseconds.
	* @param PingTimeoutDisabled		Whether or not the client socket should disconnect itself when the ping times out
	* @param TimestampRequests			Whether or not to add a timestamp to the WebSocket.
	* @param TimestampParam				The query parameter name to use to hold the timestamp.
	* @param AuthTokenName				The name of the JWT auth token (provided to the authEngine - By default this is the Storage variable name); defaults to 'socketCluster.authToken'.
	* @param RejectUnauthorized			Set this to false during debugging - Otherwise client connection will fail when using self-signed certificates.
	* @param AutoSubscribeOnConnect		This is true by default. If you set this to false, then the socket will not automatically try to subscribe to pending subscriptions on connect - Instead, you will have to manually invoke the processSubscriptions callback from inside the 'connect' event handler on the client side. See SCSocket Client API. This gives you more fine-grained control with regards to when pending subscriptions are processed after the socket connection is established (or re-established).
	* @param ChannelPrefix				The prefix of the channel names
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Query", AdvancedDisplay = "Query, AuthEngine, CodecEngine, ProtocolVersion, AckTimeOut, AutoConnect, AutoReconnect, ReconnectInitialDelay, ReconnectRandomness, ReconnectMultiplier, ReconnectMaxDelay, PubSubBatchDuration, ConnectTimeout, PingTimeoutDisabled, TimestampRequests, TimestampParam, AuthTokenName, RejectUnauthorized, AutoSubscribeOnConnect, ChannelPrefix"), Category = "SocketCluster|Client")
	static USCClientSocket* BP_Create(
		const UObject* WorldContextObject,
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
		const float ConnectTimeOut = 20.0f,
		const bool PingTimeoutDisabled = false,
		const bool TimestampRequests = false,
		const FString& TimestampParam = FString(TEXT("t")),
		const FString& AuthTokenName = FString(TEXT("socketCluster.authToken")),
		const bool RejectUnauthorized = true,
		const bool AutoSubscribeOnConnect = true,
		const FString& ChannelPrefix = FString(TEXT(""))
	);
};
