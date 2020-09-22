// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCSocket.h"
#include "SCSocketModule.h"

void USCSocket::CreateWebSocket(FString Url)
{
	Socket = FWebSocketsModule::Get().CreateWebSocket(Url, TEXT("ws"));

	Socket->OnConnected().AddLambda([&]() -> void {
#if !UE_BUILD_SHIPPING
		UE_LOG(SCSocket, Log, TEXT("Connected To Server."));
#endif
		if (onopen)
		{
			onopen();
		}
	});

	Socket->OnConnectionError().AddLambda([&](const FString& Error) -> void {
#if !UE_BUILD_SHIPPING
		UE_LOG(SCSocket, Error, TEXT("Error : %s"), *Error);
#endif
		if (onerror)
		{
			onerror(Error);
		}
	});

	Socket->OnClosed().AddLambda([&](int32 StatusCode, const FString& Reason, bool bWasClean) -> void {
#if !UE_BUILD_SHIPPING
		UE_LOG(SCSocket, Warning, TEXT("Connection Closed : %s StatusCode : %d bWasClean : %f"), *Reason, StatusCode, (bWasClean ? TEXT("True") : TEXT("False")));
#endif
		if (onclose)
		{
			onclose(StatusCode, Reason, bWasClean);
		}
	});

	Socket->OnMessage().AddLambda([&](const FString& Message) -> void {
#if !UE_BUILD_SHIPPING
		UE_LOG(SCSocket, Log, TEXT("Received : %s"), *Message);
#endif
		if (onmessage)
		{
			onmessage(Message);
		}
	});

/*
	Socket->OnRawMessage().AddLambda([&](const void* Data, SIZE_T Size, SIZE_T BytesRemaning) -> void {
		// convert binary to string send to onmessage

		UE_LOG(SCSocket, Log, TEXT("Received Raw : "));
		if (onmessage)
		{
			//do something
		}
		
	});
*/
	Socket->Connect();
}

void USCSocket::Send(FString Message)
{
	if (!Socket->IsConnected())
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(SCSocket, Warning, TEXT("Failed to send message, Socket not connected."));
#endif
		return;
	}

	Socket->Send(Message);

#if !UE_BUILD_SHIPPING
	UE_LOG(SCSocket, Log, TEXT("Send : %s"), *Message);
#endif
}

void USCSocket::Close()
{
	if (!Socket->IsConnected())
	{
		return;
	}

	Socket->Close();

#if !UE_BUILD_SHIPPING
	UE_LOG(SCSocket, Log, TEXT("Closed Connection."));
#endif
}
