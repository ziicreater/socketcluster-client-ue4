// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "Tickable.h"
#include "Sockets.h"
#include "Delegates/DelegateCombinations.h"
#include "Runtime/Engine/Public/TimerManager.h"

#include "SCAuthEngine.h"
#include "SCCodecEngine.h"

#include "SCEventObject.h"

#include "SCChannel.h"

#include "SCResponse.h"

#include "SCErrors.h"

#include "SCClientSocket.generated.h"

class USCTransport;


USTRUCT(BlueprintType)
struct FSocketClusterKeyValue
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString value;
};

/** The states of the socket */
UENUM(BlueprintType, DisplayName = "SocketClusterState")
enum class ESocketClusterState : uint8
{
	CLOSED,
	CONNECTING,
	OPEN
};

/** The authstates of the socket */
UENUM(BlueprintType, DisplayName = "SocketClusterAuthState")
enum class ESocketClusterAuthState : uint8
{
	AUTHENTICATED,
	UNAUTHENTICATED
};

/** */
UENUM()
enum class ESocketClusterLocalEvents : uint8
{
	connect,
	connectAbort,
	close,
	disconnect,
	message,
	error,
	raw,
	kickOut,
	subscribe,
	unsubscribe,
	subscribeStateChange,
	authStateChange,
	authenticate,
	deauthenticate,
	removeAuthToken,
	subscribeRequest
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSCOnConnecting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnConnect, const USCJsonObject*, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCOnDisconnect, const int32, Code, const FString&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCOnConnectAbort, const int32, Code, const FString&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCOnClose, const int32, Code, const FString&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnError, const USCJsonObject*, Error);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnAuthenticate, const FString&, SignedAuthToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnDeauthenticate, const FString&, OldSignedToken);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnAuthStateChange, const USCJsonObject*, StateChangeData);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnRemoveAuthToken, const FString&, OldToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnSubscribeStateChange, const USCJsonObject*, StateChangeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCOnSubscribe, const FString&, ChannelName, const USCJsonObject*, SubscriptionOptions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSCOnSubscribeFail, const USCJsonObject*, Error, const FString&, ChannelName, const USCJsonObject*, SubscriptionOptions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCOnSubscribeRequest, const FString&, ChannelName, const USCJsonObject*, SubscriptionOptions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnUnSubscribe, const FString&, ChannelName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnRaw, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCOnMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCOnKickOut, const FString&, Message, const FString&, ChannelName);


/**
* The SocketCluster Client Socket
*/
UCLASS(Blueprintable, BlueprintType, DisplayName = "SocketCluster Client Socket")
class SCCLIENT_API USCClientSocket : public UObject
{

	GENERATED_BODY()

	USCClientSocket();

	virtual void BeginDestroy() override;

	//////////////////////////////////////////////////////////////////////////
	// Variables
	//////////////////////////////////////////////////////////////////////////

	/** The current CodecEngine used */
	UPROPERTY()
	USCCodecEngine* codec;

	/** The current AuthEngine used */
	UPROPERTY()
	USCAuthEngine* auth;

	/** The current Transport */
	UPROPERTY()
	USCTransport* transport;

	FString clientId;

	/** A boolean wich indicates if this client is active */
	bool active;

	/** The socket id */
	FString id;

	/** The current state of the socket */
	ESocketClusterState state;

	/** The last known authentication state of the socket */
	ESocketClusterAuthState authState;

	/** The signed auth token currently associated with the socket (encoded and signed in the JWT format) */
	FString signedAuthToken;

	/** The auth token currently associated with the socket */
	UPROPERTY()
	USCJsonObject* authToken;

	/** The auth token name currently associated with the socket */
	FString authTokenName;

	/** A boolean which indicates if the socket is about to be automatically reconnected */
	bool pendingReconnect;

	/** The number of milliseconds until the next reconnection attempt is executed */
	float pendingReconnectTimeout;

	/** A boolean which indicates if we are currently preparing for pending subscriptions */
	bool preparingPendingSubscriptions;

	/** A float for the Timeout of the connect event */
	float connectTimeout;

	/** A float for the Timeout of the callback event */
	float ackTimeout;

	/** */
	FString channelPrefix;

	/** How many seconds to wait without receiving a ping before closing the socket */
	float pingTimeout;

	/** A boolean which indicates if pingTimeout should be disabled */
	bool pingTimeoutDisabled;

	/** The number of automatic connect/reconnect attempts which the socket has executed */
	int32 connectAttempts;

	/** An array which holds all the channels (USCChannel) attached to this socket */
	TMap<FString, USCChannel*> channels;

	/** An array wich holds the emit event buffer */
	TArray<USCEventObject*> _emitBuffer;

	/** An key/value for the private event handler map*/
	TMap<FString, TFunction<void(USCJsonObject*, USCResponse*)>> _privateEventHandlerMap;

	TMap<FString, TFunction<void(USCJsonObject*, USCResponse*)>> Emitter;
	
	TMultiMap<FString, TFunction<void(USCJsonObject*)>> _channelEmitter;

	TMap<FString, int32> _localEvents;

	//TMap<FString, FChannelHandler> _channelEmitter;
	//TMultiMap<FString, FChannelHandler> _channelEmitter;
	
	//TMultiMap<FString, FEventHandler> _eventEmitter;

	/** An object with all options */
	UPROPERTY()
	USCJsonObject* options;

	/** The current call id */
	int32 _cid;

	//////////////////////////////////////////////////////////////////////////
	// Timers
	//////////////////////////////////////////////////////////////////////////

	FTimerDelegate _eventTimeout;

	FTimerDelegate _reconnectTimeoutRef;
	FTimerHandle _reconnectTimeoutHandle;

public:

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnConnecting OnConnecting;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnConnect OnConnect;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnDisconnect OnDisconnect;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnConnectAbort OnConnectAbort;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnClose OnClose;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnError OnError;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnAuthenticate OnAuthenticate;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnDeauthenticate OnDeauthenticate;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnAuthStateChange OnAuthStateChange;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnRemoveAuthToken OnRemoveAuthToken;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnSubscribeStateChange OnSubscribeStateChange;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnSubscribe OnSubscribe;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnSubscribeFail OnSubscribeFail;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnSubscribeRequest OnSubscribeRequest;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnUnSubscribe OnUnSubscribe;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnRaw OnRaw;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnMessage OnMessage;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Client")
		FSCOnKickOut OnKickOut;

	UPROPERTY(Transient)
		UWorld* World;

	virtual class UWorld* GetWorld() const override;

	void create(const UObject* WorldContextObject, TSubclassOf<USCAuthEngine> authEngine, TSubclassOf<USCCodecEngine> codecEngine, USCJsonObject* opts);

	/**
	* Returns the state of the socket as a enum.
	* - CLOSED
	* - CONNECTING
	* - OPEN
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get State"), Category = "SocketCluster|Client")
		ESocketClusterState getState();

	/**
	* Perform client - initiated deauthentication - Deauthenticate(logout) the current socket.The callback will receive an error as the first argument if the operation fails.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deauthenticate", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Client")
		void deauthenticateBlueprint(const FString& Callback = FString(""), UObject* CallbackTarget = nullptr);

private:

	void deauthenticateBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error);

public:

	/**
	* Perform client - initiated deauthentication - Deauthenticate(logout) the current socket.The callback will receive an error as the first argument if the operation fails.
	*/
	void deauthenticate(TFunction<void(USCJsonObject*)> callback = nullptr);
	
	/**
	* Reconnects the client socket to its origin server.
	*
	* - You don't need to call Connect manually unless you have unchecked AutoConnect at the creation of the socket
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect"), Category = "SocketCluster|Client")
		void connect();

private:

	void reconnect(int32 code, USCJsonObject* data);

public:

	/** Disconnect From The SocketCluster Server */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disconnect", HidePin = "code", AutoCreateRefTerm = "data"), Category = "SocketCluster|Client")
		void disconnect(int32 code = 1000, USCJsonObject* data = nullptr);

	/**
	* Disconnects and destroys the socket so that it can be garbage collected.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy", HidePin = "code, data", AutoCreateRefTerm = "Data"), Category = "SocketCluster|Client")
		void destroy(int32 code = 1000, USCJsonObject* data = nullptr);

private:

	void _changeToUnauthenticatedStateAndClearTokens();

	void _changeToAuthenticatedState(FString token);

	FString decodeBase64(FString encodedString);

	FString encodeBase64(FString decodedString);

	USCJsonObject* _extractAuthTokenData(FString signedAuthToken);

public:

	/**
	* Returns the auth token as a plain JavaScript object.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Auth Token"), Category = "SocketCluster|Client")
		USCJsonObject* getAuthToken();

	/**
	* Returns the signed auth token as a string in the JWT format.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Signed Auth Token"), Category = "SocketCluster|Client")
		FString getSignedAuthToken();

	/**
	* Perform client-initiated authentication - This is useful if you already have a valid encrypted auth token string 
	* and would like to use it to authenticate directly with the server (without having to ask the user to login details)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Authenticate", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Client")
		void authenticateBlueprint(const FString& signedAuthToken, const FString& Callback = FString(""), UObject* CallbackTarget = nullptr);

private:

	void authenticateBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error, USCJsonObject* data);

public:

	/**
	* Perform client-initiated authentication - This is useful if you already have a valid encrypted auth token string
	* and would like to use it to authenticate directly with the server (without having to ask the user to login details)
	*/
	void authenticate(FString signedAuthToken, TFunction<void(USCJsonObject*, USCJsonObject*)> callback = nullptr);

private:

	void _tryReconnect(float initialDelay = NULL);

	void _onSCOpen(USCJsonObject* status);

	void _onSCError(USCJsonObject* err);

	void _suspendSubscriptions();

	void _abortAllPendingEventsDueToBadConnection(FString failureType);

	void _onSCClose(int32 code, FString data, bool openAbort = false);

	void _onSCEvent(FString event, USCJsonObject* data, USCResponse* res);

	USCJsonObject* decode(FString message);

	FString encode(USCJsonObject* object);

	void _flushEmitBuffer();

	void _handleEventAckTimeout(USCEventObject* eventObject);

	void _emit(FString event, USCJsonObject* data, TFunction<void(USCJsonObject*, USCJsonObject*)> callback);

public:

	/**
	* Send some raw data to the server. This will trigger a 'raw' event on the server which will carry the provided data.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Send"), Category = "SocketCluster|Client")
		void send(const FString& data);

	/**
	* Emit the specified event on the corresponding server-side socket. Note that you cannot emit any of the reserved SCSocket events
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Emit", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Client")
		void emitBlueprint(const FString& Event, USCJsonObject* Data = nullptr, const FString& Callback = FString(""), UObject* CallbackTarget = nullptr);

private:

	void emitBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error, USCJsonObject* data);

public:

	/**
	* Emit the specified event on the corresponding server-side socket. Note that you cannot emit any of the reserved SCSocket events
	*/
	void emit(FString event, USCJsonObject* data = nullptr, TFunction<void(USCJsonObject*, USCJsonObject*)> callback = nullptr);

	/**
	* Publish data to the specified channelName. The channelName argument must be a string. 
	* The data argument can be any JSON-compatible object/array or primitive. The callback lets you check that the publish action reached the backend successfully
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Client")
		void publishBlueprint(const FString& ChannelName, USCJsonObject* Data = nullptr, const FString& Callback = FString(""), UObject* CallbackTarget = nullptr);

private:

	void publishBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error, USCJsonObject* data);

public:

	/**
	* Publish data to the specified channelName. The channelName argument must be a string.
	* The data argument can be any JSON-compatible object/array or primitive. The callback lets you check that the publish action reached the backend successfully
	*/
	void publish(FString channelName, USCJsonObject* data, TFunction<void(USCJsonObject*, USCJsonObject*)> callback = nullptr);

private:

	void _triggerChannelSubscribe(USCChannel* channel, USCJsonObject* subscriptionOptions);

	void _triggerChannelSubscribeFail(USCJsonObject* err, USCChannel* channel, USCJsonObject* subscriptionOptions);

	void _cancelPendingSubscribeCallback(USCChannel* channel);

	FString _decorateChannelName(FString channelName);

	FString _undecorateChannelName(FString decoratedChannelName);

	void _trySubscribe(USCChannel* channel);

public:

	/**
	* Subscribe to a particular channel. This function returns an SCChannel object which lets you watch for incoming data on that channel.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscribe"), Category = "SocketCluster|Client")
		USCChannel* subscribe(const FString& channelName, const bool waitForAuth = false, USCJsonObject* data = nullptr, const bool batch = false);

private:

	void _triggerChannelUnsubscribe(USCChannel* channel, ESocketClusterChannelState newState = ESocketClusterChannelState::UNSUBSCRIBED);

	void _tryUnsubscribe(USCChannel* channel);

public:

	/**
	* Unsubscribe from the specified channel. This makes any associated SCChannel object inactive. 
	* You can reactivate the SCChannel object by calling subscribe(channelName) again at a later time.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnSubscribe"), Category = "SocketCluster|Client")
		void unsubscribe(const FString& channelName);

	/**
	* Returns an SCChannel instance. This is different from subscribe() in that it will not try to subscribe to that channel. 
	* The returned channel will be inactive initially. You can call channel->subscribe() later to activate that channel when required.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Channel"), Category = "SocketCluster|Client")
		USCChannel* channel(const FString& channelName, const bool waitForAuth = false, USCJsonObject* data = nullptr, const bool batch = false);

	/**
	* This will cause SCSocket to unsubscribe that channel and remove any watchers from it. 
	* Any SCChannel object which is associated with that channelName will be disabled permanently (ready to be cleaned up by garbage collector).
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DestroyChannel"), Category = "SocketCluster|Client")
		void destroyChannel(const FString& channelName);

	/**
	* Returns an array of active channel subscriptions which this socket is bound to. If includePending is true, pending subscriptions will also be included in the list.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscriptions"), Category = "SocketCluster|Client")
		TArray<FString> subscriptions(const bool includePending = false);

	/**
	* Check if socket is subscribed to channelName. If includePending is true, pending subscriptions will also be included in the list.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "isSubscribed"), Category = "SocketCluster|Client")
		bool isSubscribed(const FString& channelName, const bool includePending = false);

private:

	void processPendingSubscriptions();

public:

	/**
	* 	Lets you watch a channel directly from the SCSocket object. The handler accepts a single data argument which holds the data which was published to the channel.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Watch", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Client")
		void watchBlueprint(const FString& channelName, const FString& Handler, UObject* HandlerTarget);

private:

	void watchBlueprintCallback(UObject* target, const FString& handler, USCJsonObject* data);

public:

	/**
	* 	Lets you watch a channel directly from the SCSocket object. The handler accepts a single data argument which holds the data which was published to the channel.
	*/
	void watch(FString channelName, TFunction<void(USCJsonObject*)> handler);

	/**
	* Stop handling data which is published on the specified channel. This is different from unsubscribe in that the socket will still receive channel data but the specified handler will no longer capture it
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnWatch"), Category = "SocketCluster|Client")
		void unwatch(const FString& channelName);

	/**
	* Get a list of functions which are currently observing this channel
	*/
	TArray<TFunction<void(USCJsonObject*)>> watchers(FString channelName);

private:

	void clearTimeout(FTimerHandle timer);

	FString queryParse(TArray<USCJsonObject*> query);

};