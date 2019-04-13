// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonObject.h"
#include "Delegates/DelegateCombinations.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCChannelOnSubscribeStateChange, const USCJsonObject*, StateChangeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCChannelOnSubscribe, const FString&, ChannelName, const USCJsonObject*, SubscriptionOptions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSCChannelOnSubscribeFail, const USCJsonObject*, Error, const FString&, ChannelName, const USCJsonObject*, SubscriptionOptions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSCChannelOnUnSubscribe, const FString&, ChannelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSCChannelOnKickOut, const FString&, Message, const FString&, ChannelName);

/**
 * The SocketCluster Channel
 */
UCLASS()
class SCCLIENT_API USCChannel : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Channel")
		FSCChannelOnSubscribeStateChange OnChannelSubscribeStateChange;
	
	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Channel")
		FSCChannelOnSubscribe OnChannelSubscribe;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Channel")
		FSCChannelOnSubscribeFail OnChannelSubscribeFail;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Channel")
		FSCChannelOnUnSubscribe OnChannelUnSubscribe;

	UPROPERTY(BlueprintAssignable, Category = "SocketCluster|Channel")
		FSCChannelOnKickOut OnChannelKickOut;

	int32 _pendingSubscriptionCid;

	/** The name of the channel */
	FString _name;

	/** The current state of the channel */
	ESocketClusterChannelState _state;

	/** The current client */
	UPROPERTY()
		USCClientSocket* _client;

	/** The channel options assigned to this channel */
	UPROPERTY()
		USCJsonObject* _options;

	bool _waitForAuth;

	bool _batch;

	USCJsonObject* _data;

	static USCChannel* create(FString name, USCClientSocket* client, USCJsonObject* options);

	void setOptions(USCJsonObject* options);

	/**
	* Returns the state of the channel as a enum
	* - SUBSCRIBED
	* - PENDING
	* - UNSUBSCRIBED
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get State"), Category = "SocketCluster|Channel")
		ESocketClusterChannelState getState();

	/**
	* Activate this channel so that it will receive all data published to it from the backend
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscribe"), Category = "SocketCluster|Channel")
		void subscribe(const bool waitForAuth = false, USCJsonObject* data = nullptr, const bool batch = false);

	/**
	* Deactivate this channel
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnSubscribe"), Category = "SocketCluster|Channel")
		void unsubscribe();

	/**
	* Check whether or not this channel is active (subscribed to the backend). The includePending argument is optional; if true, the function will return true if the channel is in a pending state
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "isSubscribed"), Category = "SocketCluster|Channel")
		bool isSubscribed(const bool includePending = false);

	/**
	* Publish data to this channel
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Channel")
		void publishBlueprint(USCJsonObject* Data = nullptr, const FString& Callback = FString(""), UObject* CallbackTarget = nullptr);

	/**
	* Publish data to this channel
	*/
	void publish(USCJsonObject* data = nullptr, TFunction<void(USCJsonObject*, USCJsonObject*)> callback = nullptr);

	/**
	* Capture any data which is published to this channel.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Watch", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Channel")
		void watchBlueprint(const FString& Handler, UObject* HandlerTarget);

	/**
	* Capture any data which is published to this channel.
	*/
	void watch(TFunction<void(USCJsonObject*)> handler);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnWatch", DefaultToSelf = "CallbackTarget"), Category = "SocketCluster|Channel")
		void unwatch();

	TArray<TFunction<void(USCJsonObject*)>> watchers();

	/**
	* Destroy the current SCChannel object - This makes it unusable and it will allow it to be garbage collected.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy"), Category = "SocketCluster|Channel")
		void destroy();
};
