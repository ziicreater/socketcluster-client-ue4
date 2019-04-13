// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Engine/World.h"

#include "SCAuthEngine.h"

#include "SCCodecEngine.h"

#include "SCJsonObject.h"

#include "SCSocket.h"

#include "SCClientSocket.h"

#include "SCEventObject.h"

#include "SCResponse.h"

#include "SCTransport.generated.h"

#define EnumToString(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetNameStringByIndex((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )

/**
 * The SocketCluster Transport
 */
UCLASS()
class SCCLIENT_API USCTransport : public UObject
{
	GENERATED_BODY()

	virtual void BeginDestroy() override;

	virtual class UWorld* GetWorld() const override;

	/** The current state */
	ESocketClusterState state;

	/** The current authEngine */
	UPROPERTY()
	USCAuthEngine* auth;

	/** The current codecEngine */
	UPROPERTY()
	USCCodecEngine* codec;

	/** The current options */
	UPROPERTY()
	USCJsonObject* options;

	/** A float for the Timeout of the connect event */
	float connectTimeout;

public:

	/** A float for the Timeout of the ping event */
	float pingTimeout;

private:

	/** A boolean which indicates if the ping Timeout is disabled */
	bool pingTimeoutDisabled;

	/** The auth token name */
	FString authTokenName;

	int32 _cid;

	/** The current socket */
	UPROPERTY()
	USCSocket* socket;

	//TArray<USCEventObject*> _callbackMap;
	TMap<int32, USCEventObject*> _callbackMap;

	TSet<USCJsonObject*> _batchSendList;

	FTimerDelegate _batchTimeout;
	FTimerHandle _batchTimeoutHandle;

	FTimerDelegate _connectTimeoutRef;
	FTimerHandle _connectTimeoutHandle;

	FTimerDelegate _pingTimeoutTicker;
	FTimerHandle _pingTimeoutTickerHandle;
	
public:

	TFunction<void(USCJsonObject* status)> onopen;

	TFunction<void(USCJsonObject* err)> onerror;

	TFunction<void(int32 code, FString data)> onclose;

	TFunction<void(int32 code, FString data)> onopenAbort;

	TFunction<void(FString event, USCJsonObject* data, USCResponse* res)> onevent;

	TFunction<void(FString message)> onraw;

	TFunction<void(FString message)> onmessage;

	void create(USCAuthEngine* authEngine, USCCodecEngine* codecEngine, USCJsonObject* opts);

	FString uri();
	
	void _onOpen();

	void _handshake(TFunction<void(USCJsonObject*, USCJsonObject*)> callback);

	void _abortAllPendingEventsDueToBadConnection(FString failureType);

	void _onClose(int32 code, FString data = "");

	void _handleEventObject(USCJsonObject* obj, FString message);

	void _onMessage(FString message);

	void _onError(USCJsonObject* err);

	void _resetPingTimeout();

	void close(int32 code = 1000, USCJsonObject* data = nullptr);

	int32 emitObject(USCEventObject* eventObject, USCJsonObject* options = nullptr);

	void _handleEventAckTimeout(USCEventObject* eventObject);

	int32 emit(FString event, USCJsonObject* data, USCJsonObject* options = nullptr, TFunction<void(USCJsonObject*, USCJsonObject*)> callback = nullptr);

	void cancelPendingResponse(int32 cid);

	USCJsonObject* decode(FString message);

	FString encode(USCJsonObject* object);

	void send(FString data);

	FString serializeObject(USCJsonObject* object);

	FString serializeObject(TSet<USCJsonObject*> object);

	void sendObjectBatch(USCJsonObject* object);

	void sendObjectSingle(USCJsonObject* object);

	void sendObject(USCJsonObject* object, USCJsonObject* options);

	int32 callIdGenerator();

	void clearTimeout(FTimerHandle timer);
};
