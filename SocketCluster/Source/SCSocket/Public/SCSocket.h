// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Templates/SharedPointer.h"
#include "SCUtility.h"
#include "SCSocket.generated.h"

/**
 * SocketCluster Socket
 */
UCLASS()
class SCSOCKET_API USCSocket : public UObject
{
	GENERATED_BODY()

private:

	TSharedPtr<IWebSocket> Socket;

public:

	ESCSocketState SocketState;

	/**
	* Called when the socket connects to the server.
	*/
	TFunction<void()> OnOpen;

	/**
	* Called when an connection error occurs on the socket.
	*/
	TFunction<void(const FString&)> OnError;

	/**
	* Called when the connection to the server is terminated.
	*/
	TFunction<void(int32, const FString&, bool)> OnClose;

	/**
	* Called when we receive a message from the server.
	*/
	TFunction<void(const FString&)> OnMessage;

	void CreateWebSocket(FString Url);

	void Send(FString Message);

	void Close();
	
};
