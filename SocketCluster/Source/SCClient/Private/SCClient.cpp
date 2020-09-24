// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCClient.h"
#include "SCJsonConvert.h"
#include "SCJsonObject.h"
#include "Interfaces/IPluginManager.h"

FString USCClient::Version()
{
	FPluginDescriptor UPlugin = IPluginManager::Get().FindPlugin(TEXT("SocketCluster"))->GetDescriptor();
	return FString(UPlugin.VersionName);
}

bool USCClient::IsUrlSecure(TSharedPtr<FJsonObject> Options)
{
	return Options->HasField("HostName") ? Options->GetStringField("HostName").StartsWith("https:", ESearchCase::IgnoreCase) : false;
}


int32 USCClient::GetPort(TSharedPtr<FJsonObject> Options, bool IsSecureDefault)
{
	bool IsSecure = Options->HasField("Secure") ? Options->GetBoolField("Secure") : IsSecureDefault;
	return Options->HasField("Port") ? Options->GetNumberField("Port") : IsSecure ? 443 : 80;
}

USCClientSocket* USCClient::Create(const UObject* WorldContextObject, TSharedPtr<FJsonObject> Options, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine)
{
	TSharedPtr<FJsonObject> Opts = MakeShareable(new FJsonObject);
	Opts->SetStringField("ClientId", FGuid::NewGuid().ToString());
	Opts->SetNumberField("Port", GetPort(Options, IsUrlSecure(Options)));
	Opts->SetStringField("HostName", "localhost");
	Opts->SetBoolField("Secure", IsUrlSecure(Options));

	Opts->SetStringField("Version", Version());

	TSharedPtr<FJsonObject> ClientOptions = USCJsonConvert::Merge(Opts, Options);

	USCClientSocket* SCClientSocket = NewObject<USCClientSocket>();
	SCClientSocket->Create(WorldContextObject, ClientOptions, AuthEngine, CodecEngine);
	return SCClientSocket;
}

USCClientSocket* USCClient::BP_Create(const UObject* WorldContextObject, USCJsonObject* Query, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine, const FString& Hostname, const bool Secure, const int32 Port, const FString& Path, const int32 ProtocolVersion, const float AckTimeOut, const bool AutoConnect, const bool AutoReconnect, const float ReconnectInitialDelay, const float ReconnectRandomness, const float ReconnectMultiplier, const float ReconnectMaxDelay, const float PubSubBatchDuration, const float ConnectTimeOut, const bool PingTimeoutDisabled, const bool TimestampRequests, const FString& TimestampParam, const FString& AuthTokenName, const bool RejectUnauthorized, const bool AutoSubscribeOnConnect, const FString& ChannelPrefix)
{
	TSharedPtr<FJsonObject> Options = MakeShareable(new FJsonObject);

	TSharedPtr<FJsonObject> QueryJson = MakeShareable(new FJsonObject);
	if (Query != NULL && Query->IsValidLowLevel())
	{
		QueryJson = Query->GetRootObject();
	}

	Options->SetObjectField("Query", QueryJson);
	Options->SetStringField("HostName", Hostname);
	Options->SetBoolField("Secure", Secure);
	Options->SetNumberField("Port", Port);
	Options->SetStringField("Path", Path);
	Options->SetNumberField("ProtocolVersion", ProtocolVersion);
	Options->SetNumberField("AckTimeOut", AckTimeOut);
	Options->SetBoolField("AutoConnect", AutoConnect);
	Options->SetBoolField("AutoReconnect", AutoReconnect);

	TSharedPtr<FJsonObject> AutoReconnectOptions = MakeShareable(new FJsonObject);
	AutoReconnectOptions->SetNumberField("InitialDelay", ReconnectInitialDelay);
	AutoReconnectOptions->SetNumberField("Randomness", ReconnectRandomness);
	AutoReconnectOptions->SetNumberField("Multiplier", ReconnectMultiplier);
	AutoReconnectOptions->SetNumberField("MaxDelay", ReconnectMaxDelay);
	Options->SetObjectField("AutoReconnectOptions", AutoReconnectOptions);

	Options->SetNumberField("PubSubBatchDuration", PubSubBatchDuration);
	Options->SetNumberField("ConnectTimeOut", ConnectTimeOut);
	Options->SetBoolField("PingTimeoutDisabled", PingTimeoutDisabled);
	Options->SetBoolField("TimestampRequests", TimestampRequests);
	Options->SetStringField("TimestampParam", TimestampParam);
	Options->SetStringField("AuthTokenName", AuthTokenName);
	Options->SetBoolField("RejectUnauthorized", RejectUnauthorized);
	Options->SetBoolField("AutoSubscribeOnConnect", AutoSubscribeOnConnect);
	Options->SetStringField("ChannelPrefix", ChannelPrefix);

	return Create(WorldContextObject, Options, AuthEngine, CodecEngine);
}
