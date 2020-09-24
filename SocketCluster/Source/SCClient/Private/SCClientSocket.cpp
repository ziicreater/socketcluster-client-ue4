// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCClientSocket.h"
#include "SCJsonConvert.h"
#include "SCClientModule.h"

USCClientSocket::USCClientSocket()
{

}

UWorld* USCClientSocket::GetWorld() const
{
	return World;
}

void USCClientSocket::VerifyDuration(float Property, FString PropertyName)
{
	float MaxTimeOut = FMath::Pow(2, 31) - 1;
	if (Property > MaxTimeOut)
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(SCClient, Error, TEXT("The %s value provided exceeded the maximum amount allowed"), *PropertyName);
#endif
	}
}

void USCClientSocket::Create(const UObject* WorldContextObject, TSharedPtr<FJsonObject> ClientOptions, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine)
{

	World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World->IsValidLowLevel())
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(SCClient, Error, TEXT("World is not valid."));
#endif
		return;
	}

	TSharedPtr<FJsonObject> DefaultOptions = MakeShareable(new FJsonObject);
	DefaultOptions->SetStringField("Path", FString(TEXT("/socketcluster/")));
	DefaultOptions->SetBoolField("Secure", false);
	DefaultOptions->SetBoolField("AutoConnect", true);
	DefaultOptions->SetBoolField("AutoReconnect", true);
	DefaultOptions->SetBoolField("AutoSubscribeOnConnect", true);
	DefaultOptions->SetNumberField("ConnectTimeOut", 20000);
	DefaultOptions->SetNumberField("AckTimeOut", 10000);
	DefaultOptions->SetStringField("TimestampRequests", FString(TEXT("t")));
	DefaultOptions->SetStringField("AuthTokenName", FString(TEXT("socketcluster.authToken")));
	//DefaultOptions->SetBoolField("BatchOnHandshake", false);
	//DefaultOptions->SetNumberField("BatchOnHandshakeDuration", 100);
	//DefaultOptions->SetNumberField("BatchInterval", 50);
	DefaultOptions->SetNumberField("ProtocolVersion", 2);

	TSharedPtr<FJsonObject> Opts = USCJsonConvert::Merge(DefaultOptions, ClientOptions);

#if !UE_BUILD_SHIPPING
	UE_LOG(SCClient, Log, TEXT("Client Socket Created : %s"), *USCJsonConvert::ToJsonString(Opts));
#endif

	Id.Empty();
	Version = Opts->HasField("Version") ? Opts->GetStringField("Version") : "";
	ProtocolVersion = Opts->GetNumberField("ProtocolVersion");
	State = ESCSocketState::CLOSED;
	AuthState = ESCAuthState::UNAUTHENTICATED;
	SignedAuthToken.Empty();
	AuthToken.Empty();
	PendingReconnect = false;
	PendingReconnectTimeOut = 0.0;
	PreparingPendingSubscriptions = false;
	ClientId = Opts->GetStringField("ClientId");

	ConnectTimeOut = Opts->GetNumberField("ConnectTimeOut");
	AckTimeOut = Opts->GetNumberField("AckTimeOut");
	ChannelPrefix = Opts->HasField("ChannelPrefix") ? Opts->GetStringField("ChannelPrefix") : "";
	AuthTokenName = Opts->GetStringField("AuthTokenName");

	Opts->SetNumberField("PingTimeOut", Opts->GetNumberField("ConnectTimeOut"));
	PingTimeOut = Opts->GetNumberField("PingTimeOut");
	PingTimeOutDisabled = Opts->HasField("PingTimeOutDisabled") ? Opts->GetBoolField("PingTimeOutDisabled") : false;
	
	VerifyDuration(ConnectTimeOut, FString(TEXT("ConnectTimeOut")));
	VerifyDuration(AckTimeOut, FString(TEXT("AckTimeOut")));
	VerifyDuration(PingTimeOut, FString(TEXT("PingTimeOut")));

	ConnectAttempts = 0;



	Options = Opts;

}

void USCClientSocket::Connect()
{
	Transport = NewObject<USCTransport>(this);
	Transport->Create(Options);
}