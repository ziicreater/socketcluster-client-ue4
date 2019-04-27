// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCErrors.h"
#include "SCErrorsModule.h"
#include "SCJsonConvert.h"
#include "SCJsonValue.h"
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

TSharedPtr<FJsonValue> USCErrors::Error(TSharedPtr<FJsonValue> error)
{
	TSharedPtr<FJsonValue> hydratedError = nullptr;
	if (error.IsValid() && error->Type == EJson::Object)
	{
		TSharedPtr<FJsonObject> err = error->AsObject();
		ESocketClusterErrors SCError = USCJsonConvert::StringToEnum<ESocketClusterErrors>("ESocketClusterErrors", err->GetStringField("name"));
		switch (SCError)
		{
			case ESocketClusterErrors::AuthTokenExpiredError:
				hydratedError = AuthTokenExpiredError(err->GetStringField("message"), err->GetStringField("expiry"));
				break;
			case ESocketClusterErrors::AuthTokenInvalidError:
				hydratedError = AuthTokenInvalidError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::AuthTokenNotBeforeError:
				hydratedError = AuthTokenNotBeforeError(err->GetStringField("message"), err->GetObjectField("data"));
				break;
			case ESocketClusterErrors::AuthTokenError:
				hydratedError = AuthTokenError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::SilentMiddlewareBlockedError:
				hydratedError = SilentMiddlewareBlockedError(err->GetStringField("message"), err->GetStringField("type"));
				break;
			case ESocketClusterErrors::InvalidActionError:
				hydratedError = InvalidActionError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::InvalidArgumentsError:
				hydratedError = InvalidArgumentsError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::InvalidOptionsError:
				hydratedError = InvalidOptionsError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::InvalidMessageError:
				hydratedError = InvalidMessageError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::SocketProtocolError:
				hydratedError = SocketProtocolError(err->GetStringField("message"), err->GetIntegerField("code"));
				break;
			case ESocketClusterErrors::ServerProtocolError:
				hydratedError = ServerProtocolError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::HTTPServerError:
				hydratedError = HTTPServerError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::ResourceLimitError:
				hydratedError = ResourceLimitError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::TimeoutError:
				hydratedError = TimeoutError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::BadConnectionError:
				hydratedError = BadConnectionError(err->GetStringField("message"), err->GetStringField("type"));
				break;
			case ESocketClusterErrors::BrokerError:
				hydratedError = BrokerError(err->GetStringField("message"));
				break;
			case ESocketClusterErrors::ProcessExitError:
				hydratedError = ProcessExitError(err->GetStringField("message"));
				break;
			default:
				hydratedError = UnknownError(err->GetStringField("message"));
				break;
		}
	}
	else
	{
		hydratedError = UnknownError(error->AsString());
	}
	return hydratedError;
}

TSharedPtr<FJsonValue> USCErrors::AuthTokenExpiredError(FString message, FString expiry)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "AuthTokenExpiredError");
	error->SetStringField("message", "message");
	error->SetStringField("expiry", "expiry");
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::AuthTokenInvalidError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "AuthTokenInvalidError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::AuthTokenNotBeforeError(FString message, TSharedPtr<FJsonObject> data)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "AuthTokenNotBeforeError");
	error->SetStringField("message", message);
	error->SetObjectField("data", data);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::AuthTokenError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "AuthTokenError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::SilentMiddlewareBlockedError(FString message, FString type)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "SilentMiddlewareBlockedError");
	error->SetStringField("message", message);
	error->SetStringField("type", type);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::InvalidActionError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "InvalidActionError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::InvalidArgumentsError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "InvalidArgumentsError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::InvalidOptionsError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "InvalidOptionsError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::InvalidMessageError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "InvalidMessageError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::SocketProtocolError(FString message, int32 code)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "SocketProtocolError");
	error->SetStringField("message", message);
	error->SetNumberField("code", code);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::ServerProtocolError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "ServerProtocolError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::HTTPServerError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "HTTPServerError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::ResourceLimitError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "ResourceLimitError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::TimeoutError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "TimeoutError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::BadConnectionError(FString message, FString type)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "BadConnectionError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::BrokerError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "BrokerError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::ProcessExitError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "ProcessExitError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}

TSharedPtr<FJsonValue> USCErrors::UnknownError(FString message)
{
	TSharedPtr<FJsonObject> error = MakeShareable(new FJsonObject);
	error->SetStringField("name", "UnknownError");
	error->SetStringField("message", message);
#if !UE_BUILD_SHIPPING
	UE_LOG(LogSCErrors, Error, TEXT("%s"), *USCJsonConvert::ToJsonString(error));
#endif
	return USCJsonConvert::ToJsonValue(error);
}