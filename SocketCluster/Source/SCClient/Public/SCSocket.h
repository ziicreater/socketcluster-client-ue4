// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "Sockets.h"
#include "SCJsonObject.h"
#include "SCClientSocket.h"
#include "SCSocket.generated.h"

/**
* The SocketCluster Socket
*/
UCLASS()
class SCCLIENT_API USCSocket : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

	/* Override BeginDestroy Event */
	virtual void BeginDestroy() override;

	/* Override Tick Event */
	virtual void Tick(float DeltaTime) override;

	/* Override IsTickable Event */
	virtual bool IsTickable() const override;

	/* Override GetStatId Event */
	virtual TStatId GetStatId() const override;

	/* Socket lws_context */
	struct lws_context* context;

	/* The lws Socket */
	struct lws* socket;

public:

	ESocketClusterState readyState;

	TFunction<void()> onopen;

	TFunction<void(const USCJsonObject*)> onclose;

	TFunction<void(const USCJsonObject*)> onmessage;

	TFunction<void(const USCJsonObject*)> onerror;

	static int ws_service_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

	static int ws_write_back(lws* wsi, const char* str, int str_size_in);
	
	void createWebSocket(FString uri, USCJsonObject* options);

	void send(FString data);

	void close(int32 code);
	
};
