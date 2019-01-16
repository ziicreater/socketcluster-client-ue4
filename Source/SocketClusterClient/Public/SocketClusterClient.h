// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "SocketClusterCodecEngine.h"
#include "SocketClusterClient.generated.h"

class USocketClusterJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisconnect);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnError);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuthenticated);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnAuthenticated);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPublish, FString, Channel, FString, Data);
//DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAuthToken, FString, Token);

/* The states of the socket */
UENUM(BlueprintType, DisplayName = "State")
enum class ESocketClusterState : uint8
{
	CLOSED,
	CONNECTING,
	OPEN
};

/* The authstates of the socket */
UENUM(BlueprintType, DisplayName = "AuthState")
enum class ESocketClusterAuthState : uint8
{
	AUTHENTICATED,
	UNAUTHENTICATED
};

/* The states of a channel */
UENUM(BlueprintType, DisplayName = "ChannelState")
enum class ESocketClusterChannelState : uint8
{
	SUBSCRIBED,
	PENDING,
	UNSUBSCRIBED
};

/* The struct of a channel */
USTRUCT()
struct FSocketClusterChannel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		FString Name;

	UPROPERTY()
		ESocketClusterChannelState State;

	UPROPERTY()
		bool WaitForAuth;

	FSocketClusterChannel()
		: State(ESocketClusterChannelState::UNSUBSCRIBED)
		, WaitForAuth(false)
	{
	}

};

USTRUCT(BlueprintType, DisplayName = "SocketCluster KeyValuePair")
struct FSocketClusterKeyValue
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category = "SocketCluster|Client", EditAnywhere, BlueprintReadWrite)
		FString key;

	UPROPERTY(Category = "SocketCluster|Client", EditAnywhere, BlueprintReadWrite)
		FString value;
};

/**
* SocketCluster Client Object
*/
UCLASS(Blueprintable, BlueprintType, DisplayName = "SocketCluster Client")
class SOCKETCLUSTERCLIENT_API USocketClusterClient : public UObject
{
	GENERATED_BODY()
		
	/* Set default settings for SocketCluster Client */
	USocketClusterClient();

public:

	//////////////////////////////////////////////////////////////////////////
	// Variables
	//////////////////////////////////////////////////////////////////////////

	/* A boolean wich indicates if this client is active */
	bool active;

	/* The socket id */
	FString id;

	/* The current state of the socket */
	ESocketClusterState state;

	/* A boolean which indicates if the socket is about to be automatically reconnected */
	bool pendingReconnect;

	/* The number of milliseconds until the next reconnection attempt is executed */
	float pendingReconnectTimeout;

	/* How many seconds to wait without receiving a ping before closing the socket */
	float pingTimeout;

	/* A boolean which indicates if pingTimeout should be disabled */
	bool pingTimeoutDisabled;

	/* A boolean which indicates if we are currently preparing for pending subscriptions */
	bool preparingPendingSubscriptions;

	/* The number of automatic connect/reconnect attempts which the socket has executed */
	int32 connectAttempts;

	/* The last known authentication state of the socket */
	ESocketClusterAuthState authState;

	/* The auth token currently associated with the socket */
	FString authToken;

	/* The signed auth token currently associated with the socket (encoded and signed in the JWT format) */
	FString signedAuthToken;

	/* The current call id */
	int32 cid;

	/* An array which holds all the channels (FSCChannel struct) attached to this socket */
	TArray<FSocketClusterChannel> channels;

	/* Send Buffer */
	TArray<FString> Buffer;

	/* Uri of the socket */
	FString uri;

	/* A KeyValuePair Array for the query */
	TArray<FSocketClusterKeyValue> query;

	/* The current CodecEngine used */
	ISocketClusterCodecEngine* codec;

	/* A boolean which indicates if AutoReconnect is enabled */
	bool bAutoReconnect;

	/* A float for the Delay of the AutoReconnect  */
	float InitialDelay;

	/* A float for the Randomness of the AutoReconnect  */
	float Randomness;

	/* A float for the Multiplier of the AutoReconnect  */
	float Multiplier;

	/* */
	float MaxDelay;

	/* A bool wich indicates if we should AutoSubscribe when OnConnect event is called  */
	bool bAutoSubscribeOnConnect;

	/* A float for the Timeout of the connect event */
	float connectTimeout;

	/* A float for the Timeout of the callback event */
	float ackTimeout;

	/* A bool wich indicates if we should request a TimeStamp */
	bool bTimestampRequests;

	/* The param of the timestamp*/
	FString timestampParam;

	/* The current context of the socket */
	struct lws_context* context;

	/* The current socket */
	struct lws* lws;

	//////////////////////////////////////////////////////////////////////////
	// Events
	//////////////////////////////////////////////////////////////////////////

	/* Emitted whenever the socket connects to the server */
	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FOnConnect OnConnect;

	/* Emitted whenever the socket has been disconnected from the server */
	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FOnDisconnect OnDisconnect;

	/* Emitted whenever a error occurs on the socket */
	//UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
	//	FOnError OnError;

	/* Emitted whenever the client is successfully authenticated by the server */
	//UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
	//	FOnAuthenticated OnAuthenticated;

	/* Emitted whenever the client becomes unauthenticated */
	//UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
	//	FOnUnAuthenticated OnUnAuthenticated;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "OnPublish"), Category = "SocketCluster|Client")
		void OnPublish(const FOnPublish& event);

	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "OnAuthToken"), Category = "SocketCluster|Client")
	//	void OnAuthToken(const FOnAuthToken& event);

	FOnPublish OnPublishCallback;

	//FOnAuthToken OnAuthTokenCallback;

	//////////////////////////////////////////////////////////////////////////
	// Functions
	//////////////////////////////////////////////////////////////////////////

	/* Override the BeginDestroy function from the object */
	virtual void BeginDestroy() override;
	
	/* Generates a new Call Id */
	int32 callIdGenerator();

	/* Create an instance of SocketClusterClient */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create", AdvancedDisplay = "3", AutoCreateRefTerm = "Query,Codec"), Category = "SocketCluster|Client")
		static USocketClusterClient* Create
		(
			const TArray<FSocketClusterKeyValue>& Query,
			UObject* Codec,
			const FString& uri = FString(TEXT("ws://localhost/socketcluster/")),
			//UObject* AuthEngine,
			//const FString& AuthTokenName = FString(TEXT("socketCluster.authToken")),
			const bool AutoConnect = true,
			const bool AutoReconnect = true,
			const float ReconnectInitialDelay = 10.0f,
			const float ReconnectRandomness = 10.0f,
			const float ReconnectMultiplier = 10.0f,
			const float ReconnectMaxDelay = 10.0f,
			const bool AutoSubscribeOnConnect = true,
			const float ConnectTimeout = 20.0f,
			const float AckTimeout = 10.0f,
			const bool TimestampRequests = false,
			const FString& TimestampParam = FString(TEXT("t"))
		);

	/* Connect to the SocketCluster Server
	* Attn.!! You don't need to call Connect() unless you uncheck AutoConnect in Create() Function! */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect"), Category = "SocketCluster|Client")
		void Connect();

	/* Disconnect from the SocketCluster Server */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disconnect"), Category = "SocketCluster|Client")
		void Disconnect();

	/* Subscribe to a channel */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscribe"), Category = "SocketCluster|Client")
		void Subscribe(const FString& channel);

	/* UnSubscribe from a channel */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnSubscribe"), Category = "SocketCluster|Client")
		void UnSubscribe(const FString& channel);

	/* Publish to a channel */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish"), Category = "SocketCluster|Client")
		void Publish(const FString& channel, const FString& data);

	void Close(int32 code, USocketClusterJsonObject* data);

	/* Send data to server function */
	void Send(USocketClusterJsonObject* data);
	
	
};