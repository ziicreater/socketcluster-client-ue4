// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Engine/World.h"
#include "SCAuthEngine.h"
#include "SCCodecEngine.h"
#include "SCJsonConvert.h"
#include "SCJsonObject.h"
#include "SCJsonValue.h"
#include "SCSocket.h"
#include "SCClientSocket.h"
#include "SCEventObject.h"
#include "SCResponse.h"
#include "SCSocket.h"
#include "SCTransport.generated.h"

/**
 * The SocketCluster Transport
 */
UCLASS()
class SCCLIENT_API USCTransport : public UObject
{
	GENERATED_BODY()

	virtual void BeginDestroy() override;

	virtual class UWorld* GetWorld() const override;

	/**
	 * The current state of the socket as a enum
	 * - CONNECTING
	 * - OPEN
	 * - CLOSED
	 */
	ESocketClusterState state;

	/** The current CodecEngine */
	UPROPERTY()
	USCCodecEngine* codec;

	/** The current AuthEngine */
	UPROPERTY()
	USCAuthEngine* auth;

	/** The current options associated with this client socket */
	TSharedPtr<FJsonObject> options;

	/** This is the timeout for the connect event in milliseconds */
	float connectTimeout;

public:

	/** This is the timeout for the ping responses from the server in milliseconds */
	float pingTimeout;

private:

	/** Whether or not the client socket should disconnect itself when the ping times out */
	bool pingTimeoutDisabled;

	/** The name of the JWT auth token (provided to the authEngine - By default this is the Storage variable name); defaults to 'socketCluster.authToken'. */
	FString authTokenName;

	/** The current callback id */
	int32 _cid;

	/** The current Socket */
	UPROPERTY()
	USCSocket* socket;

	/** The internal callback map*/
	TMap<int32, USCEventObject*> _callbackMap;

	/** The internal batch send list*/
	TArray<TSharedPtr<FJsonValue>> _batchSendList;

	/** The batch timeout reference */
	FTimerDelegate _batchTimeout;

	/** The batch timeout handler */
	FTimerHandle _batchTimeoutHandle;

	/** The connect timeout reference */
	FTimerDelegate _connectTimeoutRef;

	/** The connect timeout handler */
	FTimerHandle _connectTimeoutHandle;

	/** The ping timeout reference */
	FTimerDelegate _pingTimeoutTicker;

	/** The ping timeout handler */
	FTimerHandle _pingTimeoutTickerHandle;
	
public:

	TFunction<void(TSharedPtr<FJsonObject> status)> onopen;

	TFunction<void(TSharedPtr<FJsonValue> err)> onerror;

	TFunction<void(int32 code, FString data)> onclose;

	TFunction<void(int32 code, FString data)> onopenAbort;

	TFunction<void(FString event, TSharedPtr<FJsonValue> data, USCResponse* res)> onevent;

	void create(USCAuthEngine* authEngine, USCCodecEngine* codecEngine, TSharedPtr<FJsonObject> opts);

private:

	FString uri();
	
	void _onOpen();

	void _handshake(TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback);

	void _abortAllPendingEventsDueToBadConnection(FString failureType);

	void _onClose(int32 code, FString data = "");

	void _handleEventObject(TSharedPtr<FJsonObject> obj, FString message);

	void _onMessage(FString message);

	void _onError(TSharedPtr<FJsonValue> err);

	void _resetPingTimeout();

public:

	void close(int32 code = 1000, TSharedPtr<FJsonValue> data = nullptr);

	int32 emitObject(USCEventObject* eventObject, TSharedPtr<FJsonObject> options = nullptr);

private:

	void _handleEventAckTimeout(USCEventObject* eventObject);

public:

	int32 emit(FString event, TSharedPtr<FJsonValue> data, TSharedPtr<FJsonObject> options = nullptr, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = nullptr);

	void cancelPendingResponse(int32 cid);

	TSharedPtr<FJsonValue> decode(FString message);

	FString encode(TSharedPtr<FJsonValue> object);

	void send(FString data);

private:

	FString serializeObject(TSharedPtr<FJsonValue> object);

	void sendObjectBatch(TSharedPtr<FJsonValue> object);

	void sendObjectSingle(TSharedPtr<FJsonValue> object);

	void sendObject(TSharedPtr<FJsonValue> object, TSharedPtr<FJsonObject> options = nullptr);

	int32 callIdGenerator();

public:

	void off();

private:

	void clearTimeout(FTimerHandle timer);
};
