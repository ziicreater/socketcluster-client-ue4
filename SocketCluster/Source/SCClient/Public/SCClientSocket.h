// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SCCodecEngine.h"
#include "SCAuthEngine.h"
#include "SCTransport.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "SCUtility.h"
#include "SCClientSocket.generated.h"

/**
 * SocketCluster Client Socket 
 */
UCLASS(Blueprintable, BlueprintType)
class SCCLIENT_API USCClientSocket : public UObject
{
	GENERATED_BODY()

	USCClientSocket();

	virtual class UWorld* GetWorld() const override;

	UWorld* World;

private:

	UPROPERTY()
	USCCodecEngine* Codec;

	UPROPERTY()
	USCAuthEngine* Auth;

	UPROPERTY()
	USCTransport* Transport;

	/**
	* The current client id
	*
	* The id of the socket connection, this is empty initially and will
	* change each time a new underlying connection is made.
	*/
	FString Id;

	/**
	*/
	FString Version;

	/**
	*
	*/
	int32 ProtocolVersion;

	/**
	* The current state of the socket
	* - CONNECTING
	* - OPEN
	* - CLOSED
	*/
	ESCSocketState State;

	/**
	* The last known authentication state of the socket
	* - AUTHENTICATED
	* - UNAUTHENTICATED
	*/
	ESCAuthState AuthState;

	/**
	* The signed auth token currently associated with the socket.
	* This property will be FJsonValueNull if no token is associated with this socket.
	*/
	FString SignedAuthToken;
	

	/**
	*/
	FString AuthToken;

	/**
	*/
	bool PendingReconnect;

	/**
	*/
	float PendingReconnectTimeOut;

	/**
	*/
	bool PreparingPendingSubscriptions;

	/**
	* The id of the socket client
	* 
	* This does not change between connections.
	*/
	FString ClientId;

	/**
	*/
	float ConnectTimeOut;

	/**
	*/
	float AckTimeOut;

	/**
	*/
	FString ChannelPrefix;

	/**
	*/
	FString AuthTokenName;

	/**
	*/
	float PingTimeOut;

	/**
	*/
	bool PingTimeOutDisabled;

	/**
	*/
	int32 ConnectAttempts;
	

	

	




	

	

	TSharedPtr<FJsonObject> Options;




	/**
	* Verifies the duration against a MaxTime variable.
	* 
	* @param Property		Float to check MaxTime against.
	* @param PropertyName	Name of Property to check against.
	*/
	void VerifyDuration(float Property, FString PropertyName);

	

public:

	void Create(const UObject* WorldContextObject, TSharedPtr<FJsonObject> Options, TSubclassOf<USCAuthEngine> AuthEngine, TSubclassOf<USCCodecEngine> CodecEngine);

	UFUNCTION(BlueprintCallable, Category = "SocketCluster|Client")
	void Connect();
};
