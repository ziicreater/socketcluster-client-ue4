// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Tickable.h"
#include "SocketClusterContext.generated.h"

typedef struct lws_context sc_lws_context;
typedef struct lws_protocols sc_lws_protocols;

class USocketClusterClient;

/**
 * 
 */
UCLASS()
class SOCKETCLUSTERCLIENT_API USocketClusterContext : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:

	// Initialize SocketClusterContext Class.
	USocketClusterContext();

	// Create Context Info
	void CreateContext();

	// Override BeginDestroy Event.
	virtual void BeginDestroy() override;

	// Override Tick Event.
	virtual void Tick(float DeltaTime) override;

	// Override IsTickable Event.
	virtual bool IsTickable() const override;

	// Override GetStatId Event.
	virtual TStatId GetStatId() const override;

	// Create a new SocketClusterClient using the Connect event.
	USocketClusterClient* Connect(const FString& url);

	// WebSocket Service Callback Function
	static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

	sc_lws_context* lws_context;
	sc_lws_protocols* protocols;
	
};
