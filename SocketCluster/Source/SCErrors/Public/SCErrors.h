// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "EngineMinimal.h"
#include "UObject/NoExportTypes.h"
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

template <typename EnumType>
static FORCEINLINE EnumType StringToEnum(const FString& EnumName, const FString& String)
{
	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	if (!Enum)
	{
		return EnumType(0);
	}
	return (EnumType)Enum->GetIndexByName(FName(*String));
}

/**
 * 
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

	static USCJsonObject* Error(USCJsonObject* error);

	static USCJsonObject* AuthTokenExpiredError(FString message, FString expiry);

	static USCJsonObject* AuthTokenInvalidError(FString message);

	static USCJsonObject* AuthTokenNotBeforeError(FString message, USCJsonObject* data);

	static USCJsonObject* AuthTokenError(FString message);

	static USCJsonObject* SilentMiddlewareBlockedError(FString message, FString type);

	static USCJsonObject* InvalidActionError(FString message);

	static USCJsonObject* InvalidArgumentsError(FString message);

	static USCJsonObject* InvalidOptionsError(FString message);

	static USCJsonObject* InvalidMessageError(FString message);

	static USCJsonObject* SocketProtocolError(FString message, int32 code);

	static USCJsonObject* ServerProtocolError(FString message);

	static USCJsonObject* HTTPServerError(FString message);

	static USCJsonObject* ResourceLimitError(FString message);

	static USCJsonObject* TimeoutError(FString message);

	static USCJsonObject* BadConnectionError(FString message, FString type);

	static USCJsonObject* BrokerError(FString message);

	static USCJsonObject* ProcessExitError(FString message);

	static USCJsonObject* UnknownError(FString message);
	
};
