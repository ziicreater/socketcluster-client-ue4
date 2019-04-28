// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "SCClient.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCJsonConvert.h"
#include "SCJsonValue.h"
#include "SCJsonObject.h"
#include "SCClientSocket.h"
#include "SCClientModule.h"

FString USCClient::GetMultiplexId(TSharedPtr<FJsonObject> options)
{
	FString protocolPrefix = options->GetBoolField("secure") ? "https://" : "http://";
	FString queryString = "";
	if (options->HasField("query") && options->GetObjectField("query")->Values.Num() > 0)
	{
		for (auto Pair : options->GetObjectField("query")->Values)
		{
			TSharedPtr<FJsonValue> Value = Pair.Value;
			if (Value->Type == EJson::String)
			{
				if (queryString.IsEmpty())
				{
					queryString.Append(TEXT("?")).Append(Pair.Key).Append(TEXT("=")).Append(Value->AsString());
				}
				else
				{
					queryString.Append(TEXT("&")).Append(Pair.Key).Append(TEXT("=")).Append(Value->AsString());
				}
			}
		}
	}
	
	FString host = options->GetStringField("hostname") + ":" + FString::FromInt(options->GetIntegerField("port"));
	return protocolPrefix + host + options->GetStringField("path") + queryString;
}

TMap<FString, USCClientSocket*> USCClient::_clients;

bool USCClient::isUrlSecure(const FString& hostname)
{
	return hostname.StartsWith("https:", ESearchCase::IgnoreCase);
}

int32 USCClient::GetPort(const int32 port, const bool secure, const bool isSecureDefault)
{
	bool isSecure = secure ? isSecureDefault : secure;
	return port != 80 && port != 443 ? port : isSecure ? 433 : 80;
}

TMap<FString, USCClientSocket*> USCClient::Clients()
{
	return _clients;
}

FString USCClient::Version()
{
	return FString("0.2 Beta");
}

void USCClient::Destroy(USCClientSocket* Socket)
{
	if (Socket)
	{
		Socket->destroy();
	}
}

USCClientSocket* USCClient::Create(
	const UObject* WorldContextObject,
	USCJsonObject* Query,
	TSubclassOf<USCAuthEngine> AuthEngine,
	TSubclassOf<USCCodecEngine> CodecEngine,
	const FString& Hostname,
	const bool Secure,
	const int32 Port,
	const FString& Path,
	const ESocketClusterProtocolVersion ProtocolVersion,
	const float AckTimeOut,
	const bool AutoConnect,
	const bool AutoReconnect,
	const float ReconnectInitialDelay,
	const float ReconnectRandomness,
	const float ReconnectMultiplier,
	const float ReconnectMaxDelay,
	const float PubSubBatchDuration,
	const float ConnectTimeout,
	const bool PingTimeoutDisabled,
	const bool TimestampRequests,
	const FString& TimestampParam,
	const FString& AuthTokenName,
	const bool Multiplex,
	const bool RejectUnauthorized,
	const bool AutoSubscribeOnConnect,
	const FString& ChannelPrefix
)
{

	TSharedPtr<FJsonObject> options = MakeShareable(new FJsonObject);

	TSharedPtr<FJsonObject> query = MakeShareable(new FJsonObject);
	if (Query != nullptr)
	{
		query = Query->GetRootObject();
	}
	
	options->SetObjectField("query", query);
	options->SetStringField("hostname", Hostname);
	options->SetBoolField("secure", Secure);
	options->SetNumberField("port", GetPort(Port, Secure, isUrlSecure(Hostname)));
	options->SetStringField("path", Path);
	options->SetNumberField("protocolVersion", (uint8)ProtocolVersion);
	options->SetNumberField("ackTimeout", AckTimeOut);
	options->SetBoolField("autoConnect", AutoConnect);
	options->SetBoolField("autoReconnect", AutoReconnect);

	TSharedPtr<FJsonObject> AutoReconnectOptions = MakeShareable(new FJsonObject);
	AutoReconnectOptions->SetNumberField("initialDelay", ReconnectInitialDelay);
	AutoReconnectOptions->SetNumberField("randomness", ReconnectRandomness);
	AutoReconnectOptions->SetNumberField("multiplier", ReconnectMultiplier);
	AutoReconnectOptions->SetNumberField("maxDelay", ReconnectMaxDelay);
	options->SetObjectField("autoReconnectOptions", AutoReconnectOptions);

	/* Currently not used*/
	TSharedPtr<FJsonObject> SubscriptionRetryOptions = MakeShareable(new FJsonObject);
	options->SetObjectField("subscriptionRetryOptions", SubscriptionRetryOptions);
	
	options->SetNumberField("pubSubBatchDuration", PubSubBatchDuration);
	options->SetNumberField("connectTimeout", ConnectTimeout);
	options->SetBoolField("pingTimeoutDisabled", PingTimeoutDisabled);
	options->SetBoolField("timestampRequests", TimestampRequests);
	options->SetStringField("timestampParam", TimestampParam);
	options->SetStringField("authTokenName", AuthTokenName);
	options->SetBoolField("rejectUnauthorized", RejectUnauthorized);
	options->SetBoolField("autoSubscribeOnConnect", AutoSubscribeOnConnect);
	options->SetStringField("channelPrefix", ChannelPrefix);

	if (Multiplex == false)
	{
		options->SetStringField("clientId", FGuid::NewGuid().ToString());
		USCClientSocket* socket = NewObject<USCClientSocket>();
		socket->create(WorldContextObject, AuthEngine, CodecEngine, options);
		_clients.Add(options->GetStringField("clientId"), socket);
		return socket;
	}

	options->SetStringField("clientId", GetMultiplexId(options));

	if (_clients.Contains(options->GetStringField("clientId")))
	{
		if (options->GetBoolField("autoConnect"))
		{
			_clients.FindRef(options->GetStringField("clientId"))->connect();
		}
	}
	else
	{
		USCClientSocket* socket = NewObject<USCClientSocket>();
		socket->create(WorldContextObject, AuthEngine, CodecEngine, options);
		_clients.Add(options->GetStringField("clientId"), socket);
	}

	return _clients.FindRef(options->GetStringField("clientId"));
}
