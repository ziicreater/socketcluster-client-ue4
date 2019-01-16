// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "Sockets.h"
#include "SocketClusterClient.h"
#include "SocketClusterContext.generated.h"

/**
 * 
 */
UCLASS()
class SOCKETCLUSTERCLIENT_API USocketClusterContext : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

	/* Set default settings for SocketCluster Context */
	USocketClusterContext();
	
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


public:

	USocketClusterClient * Create(
		const TArray<FSocketClusterKeyValue>& Query,
		UObject* Codec,
		const FString& uri,
		const bool AutoConnect,
		const bool AutoReconnect,
		const float ReconnectInitialDelay,
		const float ReconnectRandomness,
		const float ReconnectMultiplier,
		const float ReconnectMaxDelay,
		const bool AutoSubscribeOnConnect,
		const float ConnectTimeout,
		const float AckTimeout,
		const bool TimestampRequests,
		const FString& TimestampParam
	);

	/* Receive from server function */
	static int ws_service_callback(struct lws* lws, enum lws_callback_reasons reason, void* user, void* in, size_t len);

	/* Send to server function */
	static int ws_write_back(lws* wsi, const char* str, int str_size_in);

	/* Create the context of the socket */
	bool CreateContext();

};
