// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCClientBlueprint.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"

USCClientSocket* USCClientBlueprint::Create
(
	USCJsonObject* Query,
	TSubclassOf<USCAuthEngine> AuthEngine,
	TSubclassOf<USCCodecEngine> CodecEngine,
	const FString& Hostname,
	const bool Secure,
	const int32 Port,
	const FString& Path,
	const int32 ProtocolVersion,
	const float AckTimeout,
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
	if (Query->IsValidLowLevel())
	{
		query = Query->GetRootObject();
	}
	
	options->SetObjectField("Query", query);
	options->SetStringField("Hostname", Hostname);
	options->SetBoolField("Secure", Secure);
	options->SetNumberField("Port", Port);
	options->SetStringField("Path", Path);
	options->SetNumberField("ProtocolVersion", ProtocolVersion);
	options->SetNumberField("AckTimeout", AckTimeout);
	options->SetBoolField("AutoConnect", AutoConnect);
	options->SetBoolField("AutoReconnect", AutoReconnect);

	TSharedPtr<FJsonObject> AutoReconnectOptions = MakeShareable(new FJsonObject);
	AutoReconnectOptions->SetNumberField("InitialDelay", ReconnectInitialDelay);
	AutoReconnectOptions->SetNumberField("Randomness", ReconnectRandomness);
	AutoReconnectOptions->SetNumberField("Multiplier", ReconnectMultiplier);
	AutoReconnectOptions->SetNumberField("MaxDelay", ReconnectMaxDelay);
	options->SetObjectField("AutoReconnectOptions", AutoReconnectOptions);

	options->SetNumberField("PubSubBatchDuration", PubSubBatchDuration);
	options->SetNumberField("ConnectTimeout", ConnectTimeout);
	options->SetBoolField("PingTimeoutDisabled", PingTimeoutDisabled);
	options->SetBoolField("TimestampRequests", TimestampRequests);
	options->SetStringField("TimestampParam", TimestampParam);
	options->SetStringField("AuthTokenName", AuthTokenName);
	options->SetBoolField("RejectUnauthorized", RejectUnauthorized);
	options->SetBoolField("AutoSubscribeOnConnect", AutoSubscribeOnConnect);
	options->SetStringField("ChannelPrefix", ChannelPrefix);

    return USCClient::Create(options, AuthEngine, CodecEngine);
}
