// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SCClientSocket.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCJsonObject.h"
#include "SCClient.generated.h"

/**
* The SocketCluster Client
* 
* This is the root standalone object for the SocketCluster client library
* You can use it to create socket connections which you can use to interact with the server in real-time.
*/
UCLASS()
class SCCLIENT_API USCClient : public UBlueprintFunctionLibrary
{

	GENERATED_BODY()

private:

	static FString GetMultiplexId(TSharedPtr<FJsonObject> options);

	static bool isUrlSecure(const FString& hostname);

	static int32 GetPort(const int32 port, const bool secure, const bool isSecureDefault);

public:

	static TMap<FString, USCClientSocket*> _clients;

	/** An list which holds all current SC clients/sockets. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clients"), Category = "SocketCluster|Client")
		static TMap<FString, USCClientSocket*> Clients();

	/** The SC client version number. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Version"), Category = "SocketCluster|Client")
		static FString Version();

	/**
	 * Completely destroys a socket. You just need to pass the socket object which you want to destroy. 
	 * This is the same as calling socket.destroy().
	 * 
	 * @param Socket	The SCClientSocket you want to destroy
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy"), Category = "SocketCluster|Client")
		static void Destroy(USCClientSocket* Socket);

	/**
	 * Creates and returns a new socket connection to the specified host
	 *
	 * @param Query					A map of key-value pairs which will be used as query parameters for the initial HTTP handshake which will initiate the WebSocket connection.
	 * @param AuthEngine				A custom engine to use for storing and loading JWT auth tokens on the client side
	 * @param CodecEngine				Lets you set a custom codec engine. This allows you to specify how data gets encoded before being sent over the wire and how it gets decoded once it reaches the other side.
	 * @param Hostname					Defaults to the current host.
	 * @param Secure					Defaults to false.
	 * @param Port						Defaults to 80 if secure is false other wise defaults to 443.
	 * @param AckTimeOut				This is the timeout for getting a response to a SCSocket emit event (when a callback is provided) in milliseconds.
	 * @param AutoConnect				Whether or not to automatically connect the socket as soon as it is created. Default is true.
	 * @param AutoReconnect			Whether or not to automatically reconnect the socket when it loses the connection.
	 * @param ReconnectInitialDelay	Initial Reconnect Delay in milliseconds.
	 * @param ReconnectRandomness		Random Reconnect Delay in milliseconds.
	 * @param ReconnectMultiplier		Reconnect Multiplier in decimal default is 1.5
	 * @param ReconnectMaxDelay		Max Reconnect Delay in milliseconds.
	 * @param PubSubBatchDuration		Defaults to null (0 milliseconds); this property affects channel subscription batching; it determines the period in milliseconds for batching multiple subscription requests together. It only affects channels that have the batch option set to true. A value of null or 0 means that all subscribe or unsubscribe requests which were made within the same call stack will be batched together
	 * @param ConnectTimeout			This is the timeout for the connect event in milliseconds.
	 * @param PingTimeoutDisabled		Whether or not the client socket should disconnect itself when the ping times out
	 * @param TimestampRequests		Whether or not to add a timestamp to the WebSocket.
	 * @param TimestampParam			The query parameter name to use to hold the timestamp.
	 * @param AuthTokenName			The name of the JWT auth token (provided to the authEngine - By default this is the Storage variable name); defaults to 'socketCluster.authToken'.
	 * @param Multiplex				Defaults to true; multiplexing allows you to reuse a socket instead of creating a second socket to the same address.
	 * @param RejectUnauthorized		Set this to false during debugging - Otherwise client connection will fail when using self-signed certificates.
	 * @param AutoSubscribeOnConnect	This is true by default. If you set this to false, then the socket will not automatically try to subscribe to pending subscriptions on connect - Instead, you will have to manually invoke the processSubscriptions callback from inside the 'connect' event handler on the client side. See SCSocket Client API. This gives you more fine-grained control with regards to when pending subscriptions are processed after the socket connection is established (or re-established).
	 * @param ChannelPrefix			The prefix of the channel names
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Query", 
		AdvancedDisplay = "Query, AuthEngine, CodecEngine, ProtocolVersion, AckTimeOut, AutoConnect, AutoReconnect, ReconnectInitialDelay, ReconnectRandomness, ReconnectMultiplier, ReconnectMaxDelay, PubSubBatchDuration, ConnectTimeout, PingTimeoutDisabled, TimestampRequests, TimestampParam, AuthTokenName, Multiplex, RejectUnauthorized, CloneData, AutoSubscribeOnConnect, ChannelPrefix"), Category = "SocketCluster|Client")
		static USCClientSocket* Create(
			const UObject* WorldContextObject,
			USCJsonObject* Query,
			TSubclassOf<USCAuthEngine> AuthEngine,
			TSubclassOf<USCCodecEngine> CodecEngine,
			const FString& Hostname = FString(TEXT("localhost")),
			const bool Secure = false,
			const int32 Port = 80,
			const FString& Path = FString(TEXT("/socketcluster/")),
			const ESocketClusterProtocolVersion ProtocolVersion = ESocketClusterProtocolVersion::SocketCluster,
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

