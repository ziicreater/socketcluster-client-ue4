// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCEventObject.h"
#include "SCChannel.h"
#include "SCResponse.h"
#include "SCErrors.h"
#include "SCJsonValue.h"
#include "SCClientSocket.generated.h"

class USCTransport;

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

/**
* The SocketCluster Client Socket
*/
UCLASS(Blueprintable, BlueprintType, DisplayName = "SCClientSocket")
class SCCLIENT_API USCClientSocket : public UObject
{

	GENERATED_BODY()

	USCClientSocket();

	virtual void BeginDestroy() override;

	/** The current CodecEngine */
	UPROPERTY()
	USCCodecEngine* codec;

	/** The current AuthEngine */
	UPROPERTY()
	USCAuthEngine* auth;

	/** The current Transport Object */
	UPROPERTY()
	USCTransport* transport;

	/** The current client id */
	FString clientId;

	/** The current state of the client */
	bool active;

	/** The socket id */
	FString id;

	/**
	* The current state of the socket as a enum
	* - CONNECTING
	* - OPEN
	* - CLOSED
	*/
	ESocketClusterState state;

	/**
	* The last known authentication state of the socket as a enum
	* - AUTHENTICATED
	* - UNAUTHENTICATED
	*/
	ESocketClusterAuthState authState;

	/** 
	* The signed auth token currently associated with the socket (encoded and signed in the JWT format). 
	* This property will be a empty string if no token is associated with this socket.
	*/
	FString signedAuthToken;

	/** 
	* The auth token (as a plain FJsonValue) currently associated with the socket. 
	* This property will be a FJsonValueNull if no token is associated with this socket.
	*/
	TSharedPtr<FJsonValue> authToken;

	/** The name of the JWT auth token (provided to the authEngine - By default this is the Storage variable name); defaults to 'socketCluster.authToken'. */
	FString authTokenName;

	/** The current state of reconnection */
	bool pendingReconnect;

	/** This is the timeout for the reconnect event in milliseconds */
	float pendingReconnectTimeout;

	/** The current state of the pending subscriptions */
	bool preparingPendingSubscriptions;

	/** This is the timeout for the connect event in milliseconds */
	float connectTimeout;

	/** This is the timeout for getting a response to a SCSocket emit event (when a callback is provided) in milliseconds */
	float ackTimeout;

	/** The prefix of the channel names */
	FString channelPrefix;

	/** This is the timeout for the ping responses from the server in milliseconds */
	float pingTimeout;

	/** Whether or not the client socket should disconnect itself when the ping times out */
	bool pingTimeoutDisabled;

	/** The connect attempts */
	int32 connectAttempts;

	/** List of channels current associated with this socket */
	TMap<FString, USCChannel*> channels;

	/** The buffer for the emit events */
	TArray<USCEventObject*> _emitBuffer;

	/** List of private events handled internally */
	TMap<FString, TFunction<void(TSharedPtr<FJsonValue>, USCResponse*)>> _privateEventHandlerMap;

	/** Event emitter to handle events */
	TMultiMap<FString, TFunction<void(TSharedPtr<FJsonValue>, USCResponse*)>> Emitter;
	
	/** Event emitter to handle channel events */
	TMultiMap<FString, TFunction<void(TSharedPtr<FJsonValue>)>> _channelEmitter;

	/** List of private events which are prohibited and for internal use only */
	TMap<FString, int32> _localEvents;

	/** The current options associated with this client socket */
	TSharedPtr<FJsonObject> options;

	/** The current callback id */
	int32 _cid;

	/** The reconnect timeout reference */
	FTimerDelegate _reconnectTimeoutRef;

	/** The reconnect timeout handler */
	FTimerHandle _reconnectTimeoutHandle;

public:

	UPROPERTY(Transient)
		UWorld* World;

	virtual class UWorld* GetWorld() const override;

	void create(const UObject* WorldContextObject, TSubclassOf<USCAuthEngine> authEngine, TSubclassOf<USCCodecEngine> codecEngine, TSharedPtr<FJsonObject> opts);

	/**
	* Returns the state of the socket as a enum.
	* - CLOSED
	* - CONNECTING
	* - OPEN
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get State"), Category = "SocketCluster|Client")
		ESocketClusterState getState();

	/**
	* Perform client-initiated deauthentication
	* Deauthenticate (logout) the current socket. The callback will receive an error as the first argument if the operation fails.
	*
	* @param callback			Optional, The name of the function to be called when the callback is received.
	* @param callbackTarget		Optional, defaults to self, The class location of the Callback function.
	*
	* Note : The Callback function needs to have at least the following parameters.
	* First Parameter : USCJsonValue* (error)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Deauthenticate", DefaultToSelf = "callbackTarget"), Category = "SocketCluster|Client")
		void deauthenticateBlueprint(const FString& callback = FString(""), UObject* callbackTarget = nullptr);

private:

	void deauthenticateBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error);

public:

	/** 
	* Perform client - initiated deauthentication - Deauthenticate(logout) the current socket.The callback will receive an error as the first argument if the operation fails. 
	* 
	* @param callback			Optional, callback(err).
	*/
	void deauthenticate(TFunction<void(TSharedPtr<FJsonValue>)> callback = nullptr);
	
	/**
	* Reconnects the client socket to its origin server.
	*
	* Note : You don't need to call Connect manually unless you have unchecked AutoConnect at the creation of the socket
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect"), Category = "SocketCluster|Client")
		void connect();

private:

	void reconnect(int32 code, TSharedPtr<FJsonValue> data);

public:

	/** 
	* Disconnect this socket from the server. This function accepts two optional arguments: A custom error code (a Number - Ideally between 4100 to 4500) and data (which can be either reason message String or an Object). 
	* 
	* @param code		Optional, error code
	* @param data		Optional, reason message
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disconnect"), Category = "SocketCluster|Client")
		void disconnectBlueprint(int32 code = 1000, USCJsonValue* data = nullptr);

	/**
	* Disconnect this socket from the server. This function accepts two optional arguments: A custom error code (a Number - Ideally between 4100 to 4500) and data (which can be either reason message String or an Object).
	*
	* @param code		Optional, error code
	* @param data		Optional, reason message
	*/
	void disconnect(int32 code = 1000, TSharedPtr<FJsonValue> data = nullptr);

	/** Disconnects and destroys the socket so that it can be garbage collected. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy", HidePin = "code, data"), Category = "SocketCluster|Client")
		void destroyBlueprint(int32 code = 1000, USCJsonValue* data = nullptr);

	/** Disconnects and destroys the socket so that it can be garbage collected. */
	void destroy(int32 code = 1000, TSharedPtr<FJsonValue> data = nullptr);

private:

	void _changeToUnauthenticatedStateAndClearTokens();

	void _changeToAuthenticatedState(FString token);

	FString decodeBase64(FString encodedString);

	FString encodeBase64(FString decodedString);

	TSharedPtr<FJsonValue> _extractAuthTokenData(FString signedAuthToken);

public:

	/** Returns the auth token as a plain JavaScript object. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Auth Token"), Category = "SocketCluster|Client")
		USCJsonValue* getAuthTokenBlueprint();

	/** Returns the auth token as a plain JavaScript object. */
	TSharedPtr<FJsonValue> getAuthToken();

	/** Returns the signed auth token as a string in the JWT format. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Signed Auth Token"), Category = "SocketCluster|Client")
		FString getSignedAuthToken();

	/**
	* Perform client-initiated authentication - This is useful if you already have a valid encrypted auth token string
	* and would like to use it to authenticate directly with the server (without having to ask the user to login details).
	* The callback will receive an error as the first argument if the operation fails.
	*
	* @param token				The token to use for authentication.
	* @param callback			Optional, The name of the function to be called when the callback is received.
	* @param callbackTarget		Optional, defaults to self, The class location of the Callback function.
	*
	* Note : The Callback function needs to have at least the following parameters.
	* First Parameter : USCJsonValue* (error)
	* Second Parameter : USCJsonValue* (authStatus)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Authenticate", DefaultToSelf = "callbackTarget"), Category = "SocketCluster|Client")
		void authenticateBlueprint(const FString& token, const FString& callback = FString(""), UObject* callbackTarget = nullptr);

private:

	void authenticateBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data);

public:

	/**
	* Perform client-initiated authentication - This is useful if you already have a valid encrypted auth token string
	* and would like to use it to authenticate directly with the server (without having to ask the user to login details)
	* The callback will receive an error as the first argument if the operation fails.
	*
	* @param token				The token to use for authentication.
	* @param callback			Optional, callback(err, authStatus);
	*/
	void authenticate(FString token, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = nullptr);

private:

	void _tryReconnect(float initialDelay = NULL);

	void _onSCOpen(TSharedPtr<FJsonValue> status);

	void _onSCError(TSharedPtr<FJsonValue> err);

	void _suspendSubscriptions();

	void _abortAllPendingEventsDueToBadConnection(FString failureType);

	void _onSCClose(int32 code, FString data, bool openAbort = false);

	void _onSCEvent(FString event, TSharedPtr<FJsonValue> data, USCResponse* res = nullptr);

	TSharedPtr<FJsonValue> decode(FString message);

	FString encode(TSharedPtr<FJsonValue> object);

	void _flushEmitBuffer();

	void _handleEventAckTimeout(USCEventObject* eventObject);

	void _emit(FString event, TSharedPtr<FJsonValue> data, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback);

public:

	/** 
	* Send some raw data to the server. This will trigger a 'raw' event on the server which will carry the provided data. 
	* 
	* @param data		Data to send to the server.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Send"), Category = "SocketCluster|Client")
		void send(const FString& data);

	/**
	* Emit the specified event on the corresponding server-side socket. Note that you cannot emit any of the reserved SCSocket events.
	*
	* @param event				The name of the event.
	* @param data				Optional, The data to send to the server.
	* @param callback			Optional, The name of the function to be called when the callback is received.
	* @param callbackTarget		Optional, defaults to self, The class location of the callback function.
	*
	* Note : The Callback function needs to have at least the following parameters.
	* First Parameter	: USCJsonValue* (error)
	* Second Parameter	: USCJsonValue* (data)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Emit", DefaultToSelf = "callbackTarget"), Category = "SocketCluster|Client")
		void emitBlueprint(const FString& event, USCJsonValue* data = nullptr, const FString& callback = FString(""), UObject* callbackTarget = nullptr);

private:

	void emitBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data);

public:

	/**
	* Emit the specified event on the corresponding server-side socket. Note that you cannot emit any of the reserved SCSocket events.
	*
	* @param event				The name of the event.
	* @param data				Optional, The data to send to the server.
	* @param callback			Optional, callback(err, data)
	*/
	void emit(FString event, TSharedPtr<FJsonValue> data = nullptr, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = nullptr);

	/**
	* Client Side Event :
	* Add a handler for a particular event (those emitted from the client). 
	* The handler is a function in the form:
	* - First Parameter	: USCJsonValue* (data)
	*
	* Server Side Event :
	* Add a handler for a particular event (those emitted from a corresponding socket on the server). 
	* The handler is a function in the form:
	* - First Parameter	: USCJsonValue* (data)
	* - Second Parameter	: USCResponse* (res)
	* The res argument is a function which can be used to send a response to the server socket which emitted the event (assuming that the server is expecting a response - I.e. A callback was provided to the emit method). 
	* The res function is in the form: res(err, message) - To send back an error, you can do either: res('This is an error') or res(1234, 'This is the error message for error code 1234'). 
	* To send back a normal non-error response: res(null, 'This is a normal response message').
	*
	* @param event					The name of the event.
	* @param handler				Optional, The name of the function to be called when the event is called.
	* @param handlerTarget			Optional, defaults to self, The class location of the handler function.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "On", DefaultToSelf = "handlerTarget"), Category = "SocketCluster|Client")
		void onBlueprint(const FString& event, const FString& handler = FString(""), UObject* handlerTarget = nullptr);

private:

	void onBlueprintHandler(const FString& event, const FString& handler, UObject* handlerTarget, TSharedPtr<FJsonValue> data, USCResponse* res);

public:

	/**
	* Client Side Event :
	* Add a handler for a particular event (those emitted from the client).
	* The handler is a function in the form: handler(data, nullptr)
	* 
	* - Note : the handler function is the same as server side event but the res will be nullptr.
	*
	* Server Side Event :
	* Add a handler for a particular event (those emitted from a corresponding socket on the server).
	* The handler is a function in the form: handler(data, res)
	* The res argument is a function which can be used to send a response to the server socket which emitted the event (assuming that the server is expecting a response - I.e. A callback was provided to the emit method).
	* The res function is in the form: res(err, message) - To send back an error, you can do either: res('This is an error') or res(1234, 'This is the error message for error code 1234').
	* To send back a normal non-error response: res(null, 'This is a normal response message').
	*
	* @param event					The name of the event.
	* @param handler				Optional, handler(data, res)
	*/
	void on(FString event, TFunction<void(TSharedPtr<FJsonValue>, USCResponse*)> handler = nullptr);

	/** 
	* Unbind a previously attached event handler. 
	*
	* @param event		The name of the event to unbind.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Off"), Category = "SocketCluster|Client")
		void off(FString event);

	/**
	* Publish data to the specified channelName. The channelName argument must be a string. 
	* The data argument can be any JSON-compatible object/array or primitive. 
	* The callback lets you check that the publish action reached the backend successfully. 
	* Callback function is in the form :
	* - First Parameter		: USCJsonValue* (error)
	* - Second Parameter	: USCJsonValue* (ackData)
	*
	* On success, the err argument will be undefined. 
	* The ackData argument will be a value which is passed back from back end middleware (undefined by default); 
	* see the example about MIDDLEWARE_PUBLISH_IN for more details. 
	*
	* @param channelName		The name of the channel to publish data to.
	* @param data				The data to send to the channel.
	* @param callback			Optional, The name of the function to be called when the callback is received.
	* @param callbackTarget		Optional, defaults to self, The class location of the Callback function.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish", DefaultToSelf = "callbackTarget"), Category = "SocketCluster|Client")
		void publishBlueprint(const FString& channelName, USCJsonValue* data = nullptr, const FString& callback = FString(""), UObject* callbackTarget = nullptr);

private:

	void publishBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data);

public:

	/**
	* Publish data to the specified channelName. The channelName argument must be a string.
	* The data argument can be any JSON-compatible object/array or primitive.
	* The callback lets you check that the publish action reached the backend successfully.
	* Callback function is in the form callback(err, ackData)
	*
	* On success, the err argument will be undefined.
	* The ackData argument will be a value which is passed back from back end middleware (undefined by default);
	* see the example about MIDDLEWARE_PUBLISH_IN for more details.
	*
	* @param channelName		The name of the channel to publish data to.
	* @param data				The data to send to the channel.
	* @param callback			Optional, callback(err, ackData)
	*/
	void publish(FString channelName, TSharedPtr<FJsonValue> data, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = nullptr);

private:

	void _triggerChannelSubscribe(USCChannel* channel, TSharedPtr<FJsonObject> subscriptionOptions);

	void _triggerChannelSubscribeFail(TSharedPtr<FJsonValue> err, USCChannel* channel, TSharedPtr<FJsonObject> subscriptionOptions);

	void _cancelPendingSubscribeCallback(USCChannel* channel);

	FString _decorateChannelName(FString channelName);

	FString _undecorateChannelName(FString decoratedChannelName);

	void _trySubscribe(USCChannel* channel);

public:

	/**
	* Subscribe to a particular channel. This function returns an SCChannel object which lets you watch for incoming data on that channel. 
	* If waitForAuth is true, the channel will wait for the socket to become authenticated before trying to subscribe to the server - These kinds of channels are sometimes known as "private channels" 
	* - Note that in this case, "authenticated" means that the client socket has received a valid JWT authToken 
	* - Read about the server-side socket.setAuthToken(tokenData) function for more details. 
	* The data property can be used to pass data along with the subscription. 
	* The batch property - It's false by default; if set to true, then the subscription request will be batched together with other subscriptions instead of being sent off immediately and individually. 
	* By the default, the batch duration is 0 (which will batch subscribe calls that are within the same call stack). 
	* Note that the pubSubBatchDuration can be specified on a per-socket basis as an option when creating/connecting the socket for the first time. 
	* Batching can result in a significant performance boost if the client needs to subscribe to a large number of channels (will also speed up socket reconnect if there are a lot of pending channels).
	* 
	* @param channelName		The name of the channel to subscribe to.
	* @param waitForAuth		Optional, Wait for the socket to become authenticated before trying to subscribe to the channel.
	* @param data				Optional, Data to send with the subscribe request.
	* @param batch				Optional, Batch subscribe request together with other subscriptions.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscribe"), Category = "SocketCluster|Client")
		USCChannel* subscribeBlueprint(const FString& channelName, const bool waitForAuth = false, USCJsonValue* data = nullptr, const bool batch = false);

	/**
	* Subscribe to a particular channel. This function returns an SCChannel object which lets you watch for incoming data on that channel.
	* If waitForAuth is true, the channel will wait for the socket to become authenticated before trying to subscribe to the server - These kinds of channels are sometimes known as "private channels"
	* - Note that in this case, "authenticated" means that the client socket has received a valid JWT authToken
	* - Read about the server-side socket.setAuthToken(tokenData) function for more details.
	* The data property can be used to pass data along with the subscription.
	* The batch property - It's false by default; if set to true, then the subscription request will be batched together with other subscriptions instead of being sent off immediately and individually.
	* By the default, the batch duration is 0 (which will batch subscribe calls that are within the same call stack).
	* Note that the pubSubBatchDuration can be specified on a per-socket basis as an option when creating/connecting the socket for the first time.
	* Batching can result in a significant performance boost if the client needs to subscribe to a large number of channels (will also speed up socket reconnect if there are a lot of pending channels).
	*
	* @param channelName		The name of the channel to subscribe to.
	* @param opts				Optional, Options for this subscribe request {waitForAuth: true, data: someCustomData, batch: true}.
	*/
	USCChannel* subscribe(const FString& channelName, TSharedPtr<FJsonObject> opts = nullptr);

private:

	void _triggerChannelUnsubscribe(USCChannel* channel, ESocketClusterChannelState newState = ESocketClusterChannelState::UNSUBSCRIBED);

	void _tryUnsubscribe(USCChannel* channel);

public:

	/**
	* Unsubscribe from the specified channel. This makes any associated SCChannel object inactive. 
	* You can reactivate the SCChannel object by calling subscribe(channelName) again at a later time.
	* 
	* @param channelName		The name of the channel to unsubscribe from.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnSubscribe"), Category = "SocketCluster|Client")
		void unsubscribe(const FString& channelName);

	/**
	* Returns an SCChannel instance. 
	* This is different from subscribe() in that it will not try to subscribe to that channel. 
	* The returned channel will be inactive initially. You can call channel->subscribe() later to activate that channel when required.
	* 
	* @param channelName		The name of the channel.
	* @param waitForAuth		Optional, Wait for the socket to become authenticated before trying to subscribe to the channel.
	* @param data				Optional, Data to send with the subscribe request.
	* @param batch				Optional, Batch subscribe request together with other subscriptions.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Channel"), Category = "SocketCluster|Client")
		USCChannel* channelBlueprint(const FString& channelName, const bool waitForAuth = false, USCJsonValue* data = nullptr, const bool batch = false);

	/**
	* Returns an SCChannel instance.
	* This is different from subscribe() in that it will not try to subscribe to that channel.
	* The returned channel will be inactive initially. You can call channel->subscribe() later to activate that channel when required.
	*
	* @param channelName		The name of the channel.
	* @param opts				Optional, Options for this channel {waitForAuth: true, data: someCustomData, batch: true}.
	*/
	USCChannel* channel(const FString& channelName, TSharedPtr<FJsonObject> opts);

	/**
	* This will cause SCSocket to unsubscribe that channel and remove any watchers from it. 
	* Any SCChannel object which is associated with that channelName will be disabled permanently (ready to be cleaned up by garbage collector).
	* 
	* @param channelName		The name of the channel to destroy.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy Channel"), Category = "SocketCluster|Client")
		void destroyChannel(const FString& channelName);

	/**
	* Returns an array of active channel subscriptions which this socket is bound to. If includePending is true, pending subscriptions will also be included in the list.
	*
	* @param includePending		Optional, Include channels in pending state.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscriptions"), Category = "SocketCluster|Client")
		TArray<FString> subscriptions(const bool includePending = false);

	/**
	* Check if socket is subscribed to channelName. If includePending is true, pending subscriptions will also be included in the list.
	* 
	* @param channelName		The name of the channel to check.
	* @param includePending		Optional, Include channels in pending state.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "isSubscribed"), Category = "SocketCluster|Client")
		bool isSubscribed(const FString& channelName, const bool includePending = false);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ProcessPendingSubscriptions"), Category = "SocketCluster|Client")
		void processPendingSubscriptions();

public:

	/**
	* Lets you watch a channel directly from the SCSocket object. The handler accepts a single data argument which holds the data which was published to the channel.
	*
	* @param channelName		The name of the channel to watch.
	* @param handler			The name of the function to be called when data is received on the channel.
	* @param handlerTarget		Optional, defaults to self, The class location of the handler function.
	*
	* The handler is a function in one of the following forms:
	* - First Parameter	: USCJsonValue* (data)
	* or
	* - First Parameter : FString (data)
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Watch", DefaultToSelf = "handlerTarget"), Category = "SocketCluster|Client")
		void watchBlueprint(const FString& channelName, const FString& handler, UObject* handlerTarget);

private:

	void watchBlueprintCallback(const FString& handler, UObject* target, TSharedPtr<FJsonValue> data);

public:

	/**
	* Lets you watch a channel directly from the SCSocket object. The handler accepts a single data argument which holds the data which was published to the channel.
	*
	* @param channelName		The name of the channel to watch.
	* @param handler			handler(data)
	*/
	void watch(FString channelName, TFunction<void(TSharedPtr<FJsonValue>)> handler);

	/**
	* Stop handling data which is published on the specified channel. This is different from unsubscribe in that the socket will still receive channel data but the specified handler will no longer capture it
	*
	* @param channelName		The name of the channel to unwatch.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnWatch"), Category = "SocketCluster|Client")
		void unwatch(const FString& channelName);

	/**
	* Get a list of functions which are currently observing this channel
	*
	* @param channelName		The name of the channel to the watchers for.
	*/
	TArray<TFunction<void(TSharedPtr<FJsonValue>)>> watchers(FString channelName);

private:

	void clearTimeout(FTimerHandle timer);

	FString queryParse(TSharedPtr<FJsonObject> query);

};