// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "SCJsonObject.h"
#include "SCSocket.generated.h"

enum class ESocketState : uint8
{
	CLOSED,
	CONNECTING,
	OPEN
};

/**
* The SocketCluster Socket
*/
UCLASS()
class SCSOCKET_API USCSocket : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override;

	virtual TStatId GetStatId() const override;

	struct lws_context* context;

	struct lws* socket;

public:

	ESocketState readyState;

	TFunction<void()> onopen;

	TFunction<void(const TSharedPtr<FJsonObject>)> onclose;

	TFunction<void(const TSharedPtr<FJsonObject>)> onmessage;

	TFunction<void(const TSharedPtr<FJsonValue>)> onerror;

	static int ws_service_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

	static int ws_write_back(lws* wsi, const char* str, int str_size_in);

	void createWebSocket(FString uri, TSharedPtr<FJsonObject> options);

	void send(FString data);

	void close(int32 code);

};


