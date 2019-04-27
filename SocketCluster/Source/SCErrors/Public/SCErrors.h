// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonValue.h"
#include "SCJsonObject.h"
#include "SCErrors.generated.h"

UENUM()
enum class ESocketClusterErrors : uint8
{
	AuthTokenExpiredError,
	AuthTokenInvalidError,
	AuthTokenNotBeforeError,
	AuthTokenError,
	SilentMiddlewareBlockedError,
	InvalidActionError,
	InvalidArgumentsError,
	InvalidOptionsError,
	InvalidMessageError,
	SocketProtocolError,
	ServerProtocolError,
	HTTPServerError,
	ResourceLimitError,
	TimeoutError,
	BadConnectionError,
	BrokerError,
	ProcessExitError,
	UnknownError
};

/**
 * The SocketCluster Errors
 */
UCLASS()
class SCERRORS_API USCErrors : public UObject
{
	GENERATED_BODY()

	static TMap<int32, FString> socketProtocolErrorStatusesLoader();

	static TMap<int32, FString> socketProtocolIgnoreStatusesLoader();
	
public:

	static TMap<int32, FString> socketProtocolErrorStatuses;

	static TMap<int32, FString> socketProtocolIgnoreStatuses;

	static TSharedPtr<FJsonValue> Error(TSharedPtr<FJsonValue> error);

	static TSharedPtr<FJsonValue> AuthTokenExpiredError(FString message, FString expiry);

	static TSharedPtr<FJsonValue> AuthTokenInvalidError(FString message);

	static TSharedPtr<FJsonValue> AuthTokenNotBeforeError(FString message, TSharedPtr<FJsonObject> data);

	static TSharedPtr<FJsonValue> AuthTokenError(FString message);

	static TSharedPtr<FJsonValue> SilentMiddlewareBlockedError(FString message, FString type);

	static TSharedPtr<FJsonValue> InvalidActionError(FString message);

	static TSharedPtr<FJsonValue> InvalidArgumentsError(FString message);

	static TSharedPtr<FJsonValue> InvalidOptionsError(FString message);

	static TSharedPtr<FJsonValue> InvalidMessageError(FString message);

	static TSharedPtr<FJsonValue> SocketProtocolError(FString message, int32 code);

	static TSharedPtr<FJsonValue> ServerProtocolError(FString message);

	static TSharedPtr<FJsonValue> HTTPServerError(FString message);

	static TSharedPtr<FJsonValue> ResourceLimitError(FString message);

	static TSharedPtr<FJsonValue> TimeoutError(FString message);

	static TSharedPtr<FJsonValue> BadConnectionError(FString message, FString type);

	static TSharedPtr<FJsonValue> BrokerError(FString message);

	static TSharedPtr<FJsonValue> ProcessExitError(FString message);

	static TSharedPtr<FJsonValue> UnknownError(FString message);
	
};
