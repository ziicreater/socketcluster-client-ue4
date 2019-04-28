// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonValue.h"
#include "SCJsonObject.h"
#include "SCChannel.generated.h"

class USCClientSocket;

/** SocketCluster Channel State */
UENUM(BlueprintType, DisplayName = "SCChannelState")
enum class ESocketClusterChannelState : uint8
{
	SUBSCRIBED,
	PENDING,
	UNSUBSCRIBED
};

/**
 * The SocketCluster Channel
 */
UCLASS(Blueprintable, BlueprintType, DisplayName = "SCChannel")
class SCCLIENT_API USCChannel : public UObject
{
	GENERATED_BODY()

public:

	/** Event emitter to handle events */
	TMultiMap<FString, TFunction<void(TSharedPtr<FJsonValue>)>> Emitter;

	/** The channel's name */
	FString channel_name;

	/**
	* Returns the state of the channel as enum
	* - SUBSCRIBED
	* - PENDING
	* - UNSUBSCRIBED
	*/
	ESocketClusterChannelState channel_state;

	/** The client associated with this channel */
	UPROPERTY()
	USCClientSocket* channel_client;

	/** The pending subscription call id if in pending mode */
	int32 channel_pendingSubscriptionCid;

	/** The current options associated with this channel */
	TSharedPtr<FJsonObject> channel_options;

	/** A boolean which indicates whether or not this channel will wait for socket authentication before subscribing to the server. It will remain in the 'pending' state until the socket becomes authenticated. */
	bool channel_waitForAuth;

	/** A boolean which indicates whether or not this channel's subscribe and unsubscribe requests will be batched together with that of other channels (for a performance boost). If false, then the subscribe/unsubscribe requests will be sent individually and immediately (assuming that the socket is online) instead of batched asynchronously. This option is false by default. */
	bool channel_batch;

	/** The data associated with this channel */
	TSharedPtr<FJsonValue> channel_data;

	static USCChannel* create(FString channelName, USCClientSocket* clientSocket, TSharedPtr<FJsonObject> options);

	void setOptions(TSharedPtr<FJsonObject> options);

	/** Returns the state of the channel as a enum
	* - SUBSCRIBED
	* - PENDING
	* - UNSUBSCRIBED
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get State"), Category = "SocketCluster|Channel")
		ESocketClusterChannelState getState();

	/**
	* Activate this channel so that it will receive all data published to it from the backend. 
	* - If waitForAuth is true, the channel will wait for the underlying socket to become authenticated before trying to subscribe to the server - This channel will then behave as a "private channel" 
	* - Note that in this case, "authenticated" means that the client socket has received a valid JWT authToken 
	* - Read about the server-side socket.setAuthToken(tokenData) function for more details. 
	* The data property of the options object can be used to pass data along with the subscription. 
	* 
	* @param waitForAuth		Optional, Wait for the socket to become authenticated before trying to subscribe to the channel.
	* @param data				Optional, Data to send with the subscribe request.
	* @param batch				Optional, Batch subscribe request together with other subscriptions.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscribe"), Category = "SocketCluster|Channel")
		void subscribeBlueprint(const bool waitForAuth = false, USCJsonValue* data = nullptr, const bool batch = false);

	/**
	* Activate this channel so that it will receive all data published to it from the backend.
	* You can provide an optional options object in the form {waitForAuth: true, data: someCustomData} (all properties are optional)
	* - If waitForAuth is true, the channel will wait for the underlying socket to become authenticated before trying to subscribe to the server - This channel will then behave as a "private channel"
	* - Note that in this case, "authenticated" means that the client socket has received a valid JWT authToken
	* - Read about the server-side socket.setAuthToken(tokenData) function for more details.
	* The data property of the options object can be used to pass data along with the subscription.
	*
	* @param options		Optional, Options for this subscribe request {waitForAuth: true, data: someCustomData, batch: true}.
	*/
	void subscribe(TSharedPtr<FJsonObject> options);

	/**
	* Deactivate this channel.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnSubscribe"), Category = "SocketCluster|Channel")
		void unsubscribe();

	/**
	* Check whether or not this channel is active (subscribed to the backend). 
	* The includePending argument is optional; if true, the function will return true if the channel is in a pending state
	* 
	* @param includePending		Optional, Include checking for pending state. 
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "isSubscribed"), Category = "SocketCluster|Channel")
		bool isSubscribed(const bool includePending = false);

	/**
	* Publish data to this channel.
	* 
	* Callback function is in the form :
	* - First Parameter		: USCJsonValue* (error)
	* - Second Parameter		: USCJsonValue* (ackData)
	*
	* @param data				The data to send to the channel.
	* @param callback			Optional, The name of the function to be called when the callback is received.
	* @param callbackTarget	Optional, defaults to self, The class location of the Callback function.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish", DefaultToSelf = "callbackTarget"), Category = "SocketCluster|Channel")
		void publishBlueprint(USCJsonValue* data = nullptr, const FString& callback = FString(""), UObject* callbackTarget = nullptr);

	/**
	* Publish data to this channel. The optional callback is in the form callback(err, ackData).
	*/
	void publish(TSharedPtr<FJsonValue> data = nullptr, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = nullptr);

	/**
	* Add a handler for a particular event on this channel.
	* The handler is a function in the form:
	* - First Parameter	: USCJsonValue* (data)
	*
	* @param event					The name of the event.
	* @param handler				The name of the function to be called when the event is called.
	* @param handlerTarget			Optional, defaults to self, The class location of the handler function.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "On", DefaultToSelf = "handlerTarget"), Category = "SocketCluster|Channel")
		void onBlueprint(const FString& event, const FString& handler = FString(""), UObject* handlerTarget = nullptr);

private:

	void onBlueprintHandler(const FString& event, const FString& handler, UObject* handlerTarget, TSharedPtr<FJsonValue> data);

public:

	/**
	* Add a handler for a particular event on this channel.
	* The handler is a function in the form: handler(data)
	*
	* @param event					The name of the event.
	* @param handler				handler(data)
	*/
	void on(FString event, TFunction<void(TSharedPtr<FJsonValue>)> handler = nullptr);

	/**
	* Unbind a previously attached event handler from this channel.
	*
	* @param event		The name of the event to unbind.
	*/
	void off(FString event);

	/**
	* Capture any data which is published to this channel.
	*
	* The handler is a function in one of the following forms:
	* - First Parameter	: USCJsonValue* (data)
	* or
	* - First Parameter : FString (data)
	*
	* @param handler			The name of the function to be called when data is received on the channel.
	* @param handlerTarget		Optional, defaults to self, The class location of the handler function.
	*
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Watch", DefaultToSelf = "handlerTarget"), Category = "SocketCluster|Channel")
		void watchBlueprint(const FString& handler, UObject* handlerTarget);

	/**
	* Capture any data which is published to this channel. The handler is a function in the form handler(data).
	*
	* @param handler		handler(data)
	*/
	void watch(TFunction<void(TSharedPtr<FJsonValue>)> handler);

	/** Unbind all handlers from this channel.*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnWatch"), Category = "SocketCluster|Channel")
		void unwatch();

	/** Get a list of functions which are currently observing this channel. */
	TArray<TFunction<void(TSharedPtr<FJsonValue>)>> watchers();

	/** Destroy the current SCChannel object - This makes it unusable and it will allow it to be garbage collected. */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy"), Category = "SocketCluster|Channel")
		void destroy();
};
