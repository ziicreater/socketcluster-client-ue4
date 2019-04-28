// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCChannel.h"
#include "SCJsonValue.h"
#include "SCClientSocket.h"

USCChannel* USCChannel::create(FString channelName, USCClientSocket* clientSocket, TSharedPtr<FJsonObject> options)
{
	USCChannel* channel = NewObject<USCChannel>();
	channel->Emitter.Empty();
	channel->channel_name = channelName;
	channel->channel_state = ESocketClusterChannelState::UNSUBSCRIBED;
	channel->channel_client = clientSocket;

	channel->channel_options = options;
	channel->setOptions(options);
	return channel;
}

void USCChannel::setOptions(TSharedPtr<FJsonObject> options)
{
	if (!options)
	{
		options = MakeShareable(new FJsonObject);
	}
	channel_waitForAuth = options->HasField("waitForAuth") ? options->GetBoolField("waitForAuth") : false;
	channel_batch = options->HasField("batch") ? options->GetBoolField("batch") : false;
	channel_pendingSubscriptionCid = 0;
	if (options->HasField("data") && options->GetObjectField("data").IsValid())
	{
		channel_data = USCJsonConvert::ToJsonValue(options->GetObjectField("data"));
	}
}

ESocketClusterChannelState USCChannel::getState()
{
	return channel_state;
}

void USCChannel::subscribeBlueprint(const bool waitForAuth, USCJsonValue* data, const bool batch)
{
	channel_client->subscribeBlueprint(channel_name, waitForAuth, data, batch);
}

void USCChannel::subscribe(TSharedPtr<FJsonObject> options)
{
	channel_client->subscribe(channel_name, options);
}

void USCChannel::unsubscribe()
{
	channel_client->unsubscribe(channel_name);
}

bool USCChannel::isSubscribed(const bool includePending)
{
	return channel_client->isSubscribed(channel_name, includePending);
}

void USCChannel::publishBlueprint(USCJsonValue* data, const FString& callback, UObject* callbackTarget)
{
	channel_client->publishBlueprint(channel_name, data, callback, callbackTarget);
}

void USCChannel::publish(TSharedPtr<FJsonValue> data, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{
	channel_client->publish(channel_name, data, callback);
}

void USCChannel::onBlueprint(const FString& event, const FString& handler, UObject* handlerTarget)
{
	if (!handler.IsEmpty())
	{
		on(event, [&, event, handler, handlerTarget](TSharedPtr<FJsonValue> data){
			onBlueprintHandler(event, handler, handlerTarget, data);
		});
	}
}

void USCChannel::onBlueprintHandler(const FString& event, const FString& handler, UObject* handlerTarget, TSharedPtr<FJsonValue> data)
{
	if (!handlerTarget->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("On target not found for handler function '" + handler + "'");
		return;
	}

	UFunction* Function = handlerTarget->FindFunction(FName(*handler));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("On handler function '" + handler + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();

	if (FunctionParams >= 1)
	{

		struct FDynamicArgs
		{
			USCJsonValue* Arg01 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*"))
		{
			USCJsonValue* Value = NewObject<USCJsonValue>();
			Value->SetRootValue(data);
			Args.Arg01 = Value;
			handlerTarget->ProcessEvent(Function, &Args);
		}
		else
		{
			USCErrors::InvalidArgumentsError("On handler function '" + handler + "' parameters incorrect");
			return;
		}
	}
	else
	{
		USCErrors::InvalidArgumentsError("On handler function '" + handler + "' parameters incorrect");
	}
}

void USCChannel::on(FString event, TFunction<void(TSharedPtr<FJsonValue>)> handler)
{
	Emitter.Add(event, handler);
}

void USCChannel::off(FString event)
{
	Emitter.Remove(event);
}

void USCChannel::watchBlueprint(const FString& handler, UObject* handlerTarget)
{
	channel_client->watchBlueprint(channel_name, handler, handlerTarget);
}

void USCChannel::watch(TFunction<void(TSharedPtr<FJsonValue>)> handler)
{
	channel_client->watch(channel_name, handler);
}

void USCChannel::unwatch()
{
	channel_client->unwatch(channel_name);
}

TArray<TFunction<void(TSharedPtr<FJsonValue>)>> USCChannel::watchers()
{
	return channel_client->watchers(channel_name);
}

void USCChannel::destroy()
{
	channel_client->destroyChannel(channel_name);
}


