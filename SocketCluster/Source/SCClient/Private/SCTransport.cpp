// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCTransport.h"
#include "SCClientModule.h"

UWorld* USCTransport::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void USCTransport::Create(TSharedPtr<FJsonObject> Options)
{

	State = ESCSocketState::CLOSED;
	//Auth = AuthEngine;
	//Codec = CodecEngine;
	//Options = Options;
	//ProtocolVersion = Options->GetNumberField("ProtocolVersion");
	ConnectTimeOut = Options->GetNumberField("ConnectTimeOut");

#if !UE_BUILD_SHIPPING
	UE_LOG(SCClient, Log, TEXT("ConnectTimeOut : %f"), ConnectTimeOut);
#endif

	//Socket = NewObject<USCSocket>();
	//Socket->CreateWebSocket("");

	//Socket->OnOpen = [&]()
	//{
	//	//_OnOpen();
	//};

	//Socket->OnClose = [&](int32 Code, const FString& Reason, bool bWasClean) 
	//{
	//	//_Destroy(Code, Reason);
	//};

	//Socket->OnMessage = [&](const FString& Message)
	//{
	//	//_OnMessage(Message);
	//};

	//Socket->OnError = [&](const FString& Error)
	//{
	//	if (State == ESCSocketState::CONNECTING)
	//	{
	//		//_Destroy(1006);
	//	}
	//};

	_ConnectTimeoutRef.BindLambda([&]() {
#if !UE_BUILD_SHIPPING
		UE_LOG(SCClient, Log, TEXT("Connection Timer Called."));
#endif
	});
	GetWorld()->GetTimerManager().SetTimer(_ConnectTimeoutHandle, _ConnectTimeoutRef, ConnectTimeOut, false);
	
}