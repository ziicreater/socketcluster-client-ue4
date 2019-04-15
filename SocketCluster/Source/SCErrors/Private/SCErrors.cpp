// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCErrors.h"
#include "SCErrorsModule.h"
#include "SCJsonConvert.h"
#include "SCJsonObject.h" 


TMap<int32, FString> USCErrors::socketProtocolErrorStatuses = USCErrors::socketProtocolErrorStatusesLoader();
TMap<int32, FString> USCErrors::socketProtocolErrorStatusesLoader()
{
	TMap<int32, FString> list;
	list.Add(1001, "Socket was disconnected");
	list.Add(1002, "A WebSocket protocol error was encountered");
	list.Add(1003, "Server terminated socket because it received invalid data");
	list.Add(1005, "Socket closed without status code");
	list.Add(1006, "Socket hung up");
	list.Add(1007, "Message format was incorrect");
	list.Add(1008, "Encountered a policy violation");
	list.Add(1009, "Message was too big to process");
	list.Add(1010, "Client ended the connection because the server did not comply with extension requirements");
	list.Add(1011, "Server encountered an unexpected fatal condition");
	list.Add(4000, "Server ping timed out");
	list.Add(4001, "Client pong timed out");
	list.Add(4002, "Server failed to sign auth token");
	list.Add(4003, "Failed to complete handshake");
	list.Add(4004, "Client failed to save auth token");
	list.Add(4005, "Did not receive #handshake from client before timeout");
	list.Add(4006, "Failed to bind socket to message broker");
	list.Add(4007, "Client connection establishment timed out");
	list.Add(4008, "Server rejected handshake from client");
	return list;
}

TMap<int32, FString> USCErrors::socketProtocolIgnoreStatuses = USCErrors::socketProtocolIgnoreStatusesLoader();
TMap<int32, FString> USCErrors::socketProtocolIgnoreStatusesLoader()
{
	TMap<int32, FString> list;
	list.Add(1000, "Socket closed normally");
	list.Add(1001, "Socket hung up");
	return list;
}

USCJsonObject* USCErrors::Error(USCJsonObject* error)
{
	USCJsonObject* hydratedError = nullptr;
	if (error)
	{
		ESocketClusterErrors SCError = USCJsonConvert::StringToEnum<ESocketClusterErrors>("ESocketClusterErrors", error->GetStringField("name"));
		switch (SCError)
		{
			case ESocketClusterErrors::AuthTokenExpiredError:
				hydratedError = AuthTokenExpiredError(error->GetStringField("message"), error->GetStringField("expiry"));
				break;
			case ESocketClusterErrors::AuthTokenInvalidError:
				hydratedError = AuthTokenInvalidError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::AuthTokenNotBeforeError:
				hydratedError = AuthTokenNotBeforeError(error->GetStringField("message"), error->GetObjectField("data"));
				break;
			case ESocketClusterErrors::AuthTokenError:
				hydratedError = AuthTokenError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::SilentMiddlewareBlockedError:
				hydratedError = SilentMiddlewareBlockedError(error->GetStringField("message"), error->GetStringField("type"));
				break;
			case ESocketClusterErrors::InvalidActionError:
				hydratedError = InvalidActionError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::InvalidArgumentsError:
				hydratedError = InvalidArgumentsError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::InvalidOptionsError:
				hydratedError = InvalidOptionsError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::InvalidMessageError:
				hydratedError = InvalidMessageError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::SocketProtocolError:
				hydratedError = SocketProtocolError(error->GetStringField("message"), error->GetIntegerField("code"));
				break;
			case ESocketClusterErrors::ServerProtocolError:
				hydratedError = ServerProtocolError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::HTTPServerError:
				hydratedError = HTTPServerError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::ResourceLimitError:
				hydratedError = ResourceLimitError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::TimeoutError:
				hydratedError = TimeoutError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::BadConnectionError:
				hydratedError = BadConnectionError(error->GetStringField("message"), error->GetStringField("type"));
				break;
			case ESocketClusterErrors::BrokerError:
				hydratedError = BrokerError(error->GetStringField("message"));
				break;
			case ESocketClusterErrors::ProcessExitError:
				hydratedError = ProcessExitError(error->GetStringField("message"));
				break;
			default:
				hydratedError = UnknownError(error->GetStringField("message"));
				break;
		}
	}
	return hydratedError;
}

USCJsonObject* USCErrors::AuthTokenExpiredError(FString message, FString expiry)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "AuthTokenExpiredError");
	error->SetStringField("message", message);
	error->SetStringField("expiry", expiry);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::AuthTokenInvalidError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "AuthTokenInvalidError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::AuthTokenNotBeforeError(FString message, USCJsonObject* data)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "AuthTokenNotBeforeError");
	error->SetStringField("message", message);
	error->SetObjectField("data", data);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::AuthTokenError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "AuthTokenError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::SilentMiddlewareBlockedError(FString message, FString type)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "SilentMiddlewareBlockedError");
	error->SetStringField("message", message);
	error->SetStringField("type", type);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::InvalidActionError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "InvalidActionError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::InvalidArgumentsError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "InvalidArgumentsError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::InvalidOptionsError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "InvalidOptionsError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::InvalidMessageError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "InvalidMessageError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::SocketProtocolError(FString message, int32 code)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "SocketProtocolError");
	error->SetStringField("message", message);
	error->SetIntegerField("code", code);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::ServerProtocolError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "ServerProtocolError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::HTTPServerError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "HTTPServerError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::ResourceLimitError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "ResourceLimitError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::TimeoutError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "TimeoutError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::BadConnectionError(FString message, FString type)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "BadConnectionError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::BrokerError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "BrokerError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::ProcessExitError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "ProcessExitError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}

USCJsonObject* USCErrors::UnknownError(FString message)
{
	USCJsonObject* error = NewObject<USCJsonObject>();
	error->SetStringField("name", "UnknownError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *error->EncodeJson());
#endif
	return error;
}