// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.


#include "SCSocket.h"
#include "SCSocketModule.h"

void USCSocket::CreateWebSocket(FString Url)
{
	Socket = FWebSocketsModule::Get().CreateWebSocket(Url, TEXT("ws"));

	Socket->OnConnected().AddLambda([&]() -> void {
		UE_LOG(SCSocket, Log, TEXT("Connected To Server."));
		if (onopen)
		{
			onopen();
		}
	});

	Socket->OnConnectionError().AddLambda([&](const FString& Error) -> void {
		UE_LOG(SCSocket, Error, TEXT("Error : %s"), *Error);
		if (onerror)
		{
			onerror(Error);
		}
	});

	Socket->OnClosed().AddLambda([&](int32 StatusCode, const FString& Reason, bool bWasClean) -> void {
		UE_LOG(SCSocket, Warning, TEXT("Connection Closed : %s StatusCode : %d bWasClean : %f"), *Reason, StatusCode, (bWasClean ? TEXT("True") : TEXT("False")));
		if (onclose)
		{
			onclose(StatusCode, Reason, bWasClean);
		}
	});

	Socket->OnMessage().AddLambda([&](const FString& Message) -> void {
		UE_LOG(SCSocket, Log, TEXT("Received : %s"), *Message);
		if (onmessage)
		{
			onmessage(Message);
		}
	});

	Socket->OnRawMessage().AddLambda([&](const void* Data, SIZE_T Size, SIZE_T BytesRemaning) -> void {
		// convert binary to string send to onmessage

		UE_LOG(SCSocket, Log, TEXT("Received Raw : "));
		if (onmessage)
		{
			//do something
		}
		
	});

	Socket->Connect();
}

void USCSocket::Send(FString Message)
{
	UE_LOG(SCSocket, Log, TEXT("Sending : %s"), *Message);
	if (!Socket->IsConnected())
	{
		UE_LOG(SCSocket, Warning, TEXT("Failed to send message, Socket not connected."));
		return;
	}

	Socket->Send(Message);
	UE_LOG(SCSocket, Log, TEXT("Send : %s"), *Message);
}

void USCSocket::Close()
{
	UE_LOG(SCSocket, Log, TEXT("Closing Connection."));
	if (!Socket->IsConnected())
	{
		UE_LOG(SCSocket, Warning, TEXT("Failed to close connection, Socket not connected."));
		return;
	}

	Socket->Close();
	UE_LOG(SCSocket, Log, TEXT("Closed Connection."));
}
