// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Templates/SharedPointer.h"
#include "SCSocket.generated.h"

enum class ESocketState : uint8
{
	CLOSED,
	CONNECTING,
	OPEN
};

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

	ESocketState SocketState;

	TFunction<void()> onopen;

	TFunction<void(const FString&)> onerror;

	TFunction<void(int32, const FString&, bool)> onclose;

	TFunction<void(const FString&)> onmessage;

	void CreateWebSocket(FString Url);

	void Send(FString Message);

	void Close();
	
};
