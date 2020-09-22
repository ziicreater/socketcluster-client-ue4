// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCClient.h"
#include "SCJsonConvert.h"

bool USCClient::IsUrlSecure(FString Hostname)
{
	return Hostname.StartsWith("https:", ESearchCase::IgnoreCase);
}


int32 USCClient::GetPort(int32 Port, bool Secure, bool IsSecureDefault)
{
	bool IsSecure = Secure ? IsSecureDefault : Secure;
	return Port != 80 && Port != 443 ? Port : IsSecure ? 443 : 80;
}

USCClientSocket* USCClient::Create(TSharedPtr<FJsonObject> Options, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine)
{
	TSharedPtr<FJsonObject> Opts = MakeShareable(new FJsonObject);
	Opts->SetStringField("ClientId", FGuid::NewGuid().ToString());
	Opts->SetNumberField("Port", GetPort(Options->GetNumberField("Port"), Options->GetBoolField("Secure"), IsUrlSecure(Options->GetStringField("Hostname"))));
	Opts->SetBoolField("Secure", IsUrlSecure(Options->GetStringField("Hostname")));

	Opts = USCJsonConvert::Merge(Opts, Options);

	USCClientSocket* SCClientSocket = NewObject<USCClientSocket>();
	SCClientSocket->Create(Opts);

	return SCClientSocket;
}
