// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCChannel.h"
#include "SCClientSocket.h"

USCChannel* USCChannel::create(FString name, USCClientSocket* client, USCJsonObject* options)
{
	USCChannel* channel = NewObject<USCChannel>();
	channel->_name = name;
	channel->_state = ESocketClusterChannelState::UNSUBSCRIBED;
	channel->_client = client;

	channel->_options = options;
	channel->setOptions(options);
	return channel;
}

void USCChannel::setOptions(USCJsonObject* options)
{
	if (!options)
	{
		options = NewObject<USCJsonObject>();
	}
	_waitForAuth = options->GetBoolField("waitForAuth") || false;
	_batch = options->GetBoolField("batch") || false;
	_pendingSubscriptionCid = 0;
	if (options->HasField("data"))
	{
		_data = options->GetObjectField("data");
	}
}

ESocketClusterChannelState USCChannel::getState()
{
	return _state;
}

void USCChannel::subscribe(const bool waitForAuth, USCJsonObject* data, const bool batch)
{
	USCJsonObject* opts = NewObject<USCJsonObject>();
	opts->SetBoolField("waitForAuth", waitForAuth);
	opts->SetObjectField("data", data);
	opts->SetBoolField("batch", batch);
	_client->subscribe(_name, opts);
}

void USCChannel::unsubscribe()
{
	_client->unsubscribe(_name);
}

bool USCChannel::isSubscribed(const bool includePending)
{
	return _client->isSubscribed(_name, includePending);
}

void USCChannel::publishBlueprint(USCJsonObject* Data, const FString& Callback, UObject* CallbackTarget)
{
	_client->publishBlueprint(_name, Data, Callback, CallbackTarget);
}

void USCChannel::publish(USCJsonObject* data, TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
{
	_client->publish(_name, data, callback);
}

void USCChannel::watchBlueprint(const FString& Handler, UObject* HandlerTarget)
{
	_client->watchBlueprint(_name, Handler, HandlerTarget);
}

void USCChannel::watch(TFunction<void(USCJsonObject*)> handler)
{
	_client->watch(_name, handler);
}

void USCChannel::unwatch()
{
	_client->unwatch(_name);
}

TArray<TFunction<void(USCJsonObject*)>> USCChannel::watchers()
{
	return _client->watchers(_name);
}

void USCChannel::destroy()
{
	_client->destroyChannel(_name);
}


