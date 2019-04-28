// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCClientSocket.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "SCJsonValue.h"
#include "SCJsonObject.h"
#include "SCJsonConvert.h"
#include "SC_Formatter.h"
#include "SCTransport.h"
#include "SCErrors.h"
#include "SCAuthEngine.h"
#include "SCDefaultAuthEngine.h"
#include "SCClient.h"
#include "SCChannel.h"
#include "SCClientModule.h"

USCClientSocket::USCClientSocket()
{
	_localEvents.Add("connect", 1);
	_localEvents.Add("connectAbort", 1);
	_localEvents.Add("close", 1);
	_localEvents.Add("disconnect", 1);
	_localEvents.Add("message", 1);
	_localEvents.Add("error", 1);
	_localEvents.Add("raw", 1);
	_localEvents.Add("kickOut", 1);
	_localEvents.Add("subscribe", 1);
	_localEvents.Add("unsubscribe", 1);
	_localEvents.Add("subscribeStateChange", 1);
	_localEvents.Add("authStateChange", 1);
	_localEvents.Add("authenticate", 1);
	_localEvents.Add("deauthenticate", 1);
	_localEvents.Add("removeAuthToken", 1);
	_localEvents.Add("subscribeRequest", 1);

	_privateEventHandlerMap.Add("#publish", [&](TSharedPtr<FJsonValue> data, USCResponse* response)
	{
		TSharedPtr<FJsonObject> dataObj = data->AsObject();
		FString undecoratedChannelName = _undecorateChannelName(dataObj->GetStringField("channel"));
		bool IsSubscribed = isSubscribed(undecoratedChannelName, true);
		if (IsSubscribed)
		{
			if (_channelEmitter.Contains(undecoratedChannelName) && _channelEmitter.FindRef(undecoratedChannelName))
			{
				_channelEmitter.FindRef(undecoratedChannelName)(USCJsonConvert::JsonStringToJsonValue(dataObj->GetStringField("data")));
			}
		}
	});
	_privateEventHandlerMap.Add("#kickOut", [&](TSharedPtr<FJsonValue> data, USCResponse* response)
	{
		TSharedPtr<FJsonObject> dataObj = data->AsObject();
		FString undecoratedChannelName = _undecorateChannelName(dataObj->GetStringField("channel"));
		USCChannel* channel = channels.FindRef(undecoratedChannelName);
		if (channel)
		{
			TSharedPtr<FJsonObject> obj = MakeShareable(new FJsonObject);
			obj->SetStringField("message", dataObj->GetStringField("message"));
			obj->SetStringField("channelName", undecoratedChannelName);
			
			if (Emitter.Contains("kickOut") && Emitter.FindRef("kickOut"))
			{
				Emitter.FindRef("kickOut")(USCJsonConvert::ToJsonValue(obj), nullptr);
			}

			if (channel->Emitter.Contains("kickOut") && channel->Emitter.FindRef("kickOut"))
			{
				channel->Emitter.FindRef("kickOut")(USCJsonConvert::ToJsonValue(obj));
			}
		
			_triggerChannelUnsubscribe(channel);
		}
	});
	_privateEventHandlerMap.Add("#setAuthToken", [&](TSharedPtr<FJsonValue> data, USCResponse* response)
	{
		if (data.IsValid())
		{
			TSharedPtr<FJsonObject> dataObj = data->AsObject();
			auth->saveToken(authTokenName, dataObj->GetStringField("token"), [&](TSharedPtr<FJsonValue> err, FString token)
			{
				if (err)
				{
					response->error(err);
					_onSCError(err);
				}
				else
				{
					_changeToAuthenticatedState(token);
					response->end();
				}
			});
		}
		else
		{
			response->error(USCErrors::InvalidMessageError("No token data provided by #setAuthToken event"));
		}
	});
	_privateEventHandlerMap.Add("#removeAuthToken", [&](TSharedPtr<FJsonValue> data, USCResponse* response)
	{
		auth->removeToken(authTokenName, [&](TSharedPtr<FJsonValue> err, FString oldToken)
		{
			if (err.IsValid())
			{
				response->error(err);
				_onSCError(err);
			}
			else
			{
				if (Emitter.Contains("removeAuthToken") && Emitter.FindRef("removeAuthToken"))
				{
					Emitter.FindRef("removeAuthToken")(USCJsonConvert::ToJsonValue(oldToken), nullptr);
				}
				_changeToUnauthenticatedStateAndClearTokens();
				response->end();
			}
		});
	});
	_privateEventHandlerMap.Add("#disconnect", [&](TSharedPtr<FJsonValue> data, USCResponse* response)
	{
		TSharedPtr<FJsonObject> dataObj = nullptr;
		if (data.IsValid() && data->Type == EJson::Object)
		{
			dataObj = data->AsObject();
		}
		transport->close(dataObj->GetNumberField("code"), USCJsonConvert::JsonStringToJsonValue(dataObj->GetStringField("data")));
	});
}

void USCClientSocket::BeginDestroy()
{
	destroy();
	Super::BeginDestroy();
}

UWorld* USCClientSocket::GetWorld() const
{
	return World;
}

void USCClientSocket::create(const UObject* WorldContextObject, TSubclassOf<USCAuthEngine> authEngine, TSubclassOf<USCCodecEngine> codecEngine, TSharedPtr<FJsonObject> opts)
{

	World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		USCErrors::InvalidActionError("Unable to access current game world.");
		return;
	}

	id.Empty();
	state = ESocketClusterState::CLOSED;
	authState = ESocketClusterAuthState::UNAUTHENTICATED;
	signedAuthToken.Empty();
	authToken = MakeShareable(new FJsonValueNull);
	pendingReconnect = false;
	pendingReconnectTimeout = 0.0f;
	preparingPendingSubscriptions = false;
	clientId = opts->GetStringField("clientId");

	connectTimeout = opts->GetNumberField("connectTimeout");
	ackTimeout = opts->GetNumberField("ackTimeout");
	channelPrefix = opts->GetStringField("channelPrefix");
	authTokenName = opts->GetStringField("authTokenName");

	pingTimeout = ackTimeout;
	pingTimeoutDisabled = opts->GetBoolField("pingTimeoutDisabled");
	active = true;

	connectAttempts = 0;

	_emitBuffer.Empty();
	channels.Empty();

	options = opts;

	_cid = 1;

	options->SetNumberField("callIdGenerator", _cid);

	if (options->GetBoolField("autoReconnect"))
	{
		TSharedPtr<FJsonObject> reconnectOptions = options->GetObjectField("autoReconnectOptions");
		if (!reconnectOptions->HasField("initialDelay"))
		{
			reconnectOptions->SetNumberField("initialDelay", 1.0f);
		}
		if (!reconnectOptions->HasField("randomness"))
		{
			reconnectOptions->SetNumberField("randomness", 1.0f);
		}
		if (!reconnectOptions->HasField("multiplier"))
		{
			reconnectOptions->SetNumberField("multiplier", 1.0f);
		}
		if (!reconnectOptions->HasField("maxDelay"))
		{
			reconnectOptions->SetNumberField("maxDelay", 1.0f);
		}
	}

	if (!options->HasField("subscriptionRetryOptions"))
	{
		options->SetObjectField("subscriptionRetryOptions", nullptr);
	}

	if (authEngine)
	{
		auth = authEngine->GetDefaultObject<USCAuthEngine>();
	}
	else
	{
		auth = NewObject<USCDefaultAuthEngine>();
	}

	if (codecEngine)
	{
		codec = codecEngine->GetDefaultObject<USCCodecEngine>();
	}
	else
	{
		codec = NewObject<USC_Formatter>();
	}

	options->SetStringField("query", queryParse(opts->GetObjectField("query")));

	_channelEmitter.Empty();

	if (options->GetBoolField("autoConnect"))
	{
		connect();
	}
}

ESocketClusterState USCClientSocket::getState()
{
	return state;
}

void USCClientSocket::deauthenticateBlueprint(const FString& callback, UObject* callbackTarget)
{
	if (!callback.IsEmpty())
	{
		deauthenticate([&, callback, callbackTarget, this](TSharedPtr<FJsonValue> error)
		{
			deauthenticateBlueprintCallback(callback, callbackTarget, error);
		});
	}
	else
	{
		deauthenticate();
	}
}

void USCClientSocket::deauthenticateBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error)
{
	if (!target->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("Deauthenticate target not found for callback function '" + callback + "'");
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("Deauthenticate callback function '" + callback + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();

	if (FunctionParams >= 1)
	{
		const FString& FirstParam = Properties[0]->GetCPPType();
		if (FirstParam.Equals("USCJsonValue*"))
		{
			struct FDynamicArgs
			{
				USCJsonValue* Arg01 = nullptr;
			};

			FDynamicArgs Args = FDynamicArgs();
			
			USCJsonValue* Value = NewObject<USCJsonValue>();
			Value->SetRootValue(error);

			Args.Arg01 = Value;

			target->ProcessEvent(Function, &Args);
		}
		else
		{
			USCErrors::InvalidArgumentsError("Deauthenticate callback function '" + callback + "' parameters incorrect");
		}

	}
	else
	{
		USCErrors::InvalidArgumentsError("Deauthenticate callback function '" + callback + "' parameters incorrect");
	}
}

void USCClientSocket::deauthenticate(TFunction<void(TSharedPtr<FJsonValue>)> callback)
{
	auth->removeToken(authTokenName, [&](TSharedPtr<FJsonValue> err, FString oldToken)
	{
		if (err.IsValid())
		{
			_onSCError(err);
		}
		else
		{
			if (Emitter.Contains("removeAuthToken") && Emitter.FindRef("removeAuthToken"))
			{
				Emitter.FindRef("removeAuthToken")(USCJsonConvert::ToJsonValue(oldToken), nullptr);
			}
			if (state != ESocketClusterState::CLOSED)
			{
				emit("#removeAuthToken");
			}
			_changeToUnauthenticatedStateAndClearTokens();
		}
		
		if (callback)
		{
			callback(err);
		}
	});
}

void USCClientSocket::connect()
{

	if (!active)
	{
		TSharedPtr<FJsonValue> error = USCErrors::InvalidActionError("Cannot connect a destroyed client");
		_onSCError(error);
		return;
	}

	if (state == ESocketClusterState::CLOSED)
	{
		pendingReconnect = false;
		pendingReconnectTimeout = 0.0f;
		clearTimeout(_reconnectTimeoutHandle);

		state = ESocketClusterState::CONNECTING;
		if (Emitter.Contains("connecting") && Emitter.FindRef("connecting"))
		{
			Emitter.FindRef("connecting")(nullptr, nullptr);
		}

		if (transport->IsValidLowLevel())
		{
			transport->off();
		}

		transport = NewObject<USCTransport>(this);
		transport->create(auth, codec, options);

		transport->onopen = [&](TSharedPtr<FJsonValue> status)
		{
			state = ESocketClusterState::OPEN;
			_onSCOpen(status);
		};

		transport->onerror = [&](TSharedPtr<FJsonValue> err)
		{
			_onSCError(err);
		};

		transport->onclose = [&](int32 code, FString data)
		{
			state = ESocketClusterState::CLOSED;
			_onSCClose(code, data);
		};

		transport->onopenAbort = [&](int32 code, FString data)
		{
			state = ESocketClusterState::CLOSED;
			_onSCClose(code, data, true);
		};

		transport->onevent = [&](FString event, TSharedPtr<FJsonValue> data, USCResponse* res)
		{
			_onSCEvent(event, data, res);
		};
	}
}

void USCClientSocket::reconnect(int32 code, TSharedPtr<FJsonValue> data)
{
	disconnect(code, data);
	connect();
}

void USCClientSocket::disconnectBlueprint(int32 code, USCJsonValue* data)
{
	TSharedPtr<FJsonValue> value = nullptr;
	if (data != nullptr)
	{
		value = data->GetRootValue();
	}
	else
	{
		value = MakeShareable(new FJsonValueNull);
	}
	disconnect(code, value);
}

void USCClientSocket::disconnect(int32 code, TSharedPtr<FJsonValue> data)
{
	if (state == ESocketClusterState::OPEN || state == ESocketClusterState::CONNECTING)
	{
		transport->close(code, data);
	}
	else
	{
		pendingReconnect = false;
		pendingReconnectTimeout = 0.0f;
		clearTimeout(_reconnectTimeoutHandle);
	}
}

void USCClientSocket::destroyBlueprint(int32 code, USCJsonValue* data)
{
	TSharedPtr<FJsonValue> value = nullptr;
	if (data != nullptr)
	{
		value = data->GetRootValue();
	}
	destroy(code, value);
}

void USCClientSocket::destroy(int32 code, TSharedPtr<FJsonValue> data)
{
	active = false;
	disconnect(code, data);
	USCClient::_clients.Remove(clientId);
}

void USCClientSocket::_changeToUnauthenticatedStateAndClearTokens()
{
	if (authState != ESocketClusterAuthState::UNAUTHENTICATED)
	{
		ESocketClusterAuthState oldState = authState;
		FString oldSignedToken = signedAuthToken;
		authState = ESocketClusterAuthState::UNAUTHENTICATED;
		signedAuthToken.Empty();
		authToken = nullptr;

		TSharedPtr<FJsonObject> stateChangeData = MakeShareable(new FJsonObject);
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", authState));

		if (Emitter.Contains("authStateChange") && Emitter.FindRef("authStateChange"))
		{
			Emitter.FindRef("authStateChange")(USCJsonConvert::ToJsonValue(stateChangeData), nullptr);
		}
		if (Emitter.Contains("deauthenticate") && Emitter.FindRef("deauthenticate"))
		{
			Emitter.FindRef("deauthenticate")(USCJsonConvert::ToJsonValue(oldSignedToken), nullptr);
		}
	}
}

void USCClientSocket::_changeToAuthenticatedState(FString token)
{
	signedAuthToken = token;
	authToken = _extractAuthTokenData(signedAuthToken);

	if (authState != ESocketClusterAuthState::AUTHENTICATED)
	{
		ESocketClusterAuthState oldState = authState;
		authState = ESocketClusterAuthState::AUTHENTICATED;

		TSharedPtr<FJsonObject> stateChangeData = MakeShareable(new FJsonObject);
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", authState));
		stateChangeData->SetStringField("signedAuthToken", signedAuthToken);
		stateChangeData->SetField("authToken", authToken);

		if (!preparingPendingSubscriptions)
		{
			processPendingSubscriptions();
		}

		if (Emitter.Contains("authStateChange") && Emitter.FindRef("authStateChange"))
		{
			Emitter.FindRef("authStateChange")(USCJsonConvert::ToJsonValue(stateChangeData), nullptr);
		}
	}

	if (Emitter.Contains("authenticate") && Emitter.FindRef("authenticate"))
	{
		Emitter.FindRef("authenticate")(USCJsonConvert::ToJsonValue(signedAuthToken), nullptr);
	}
}

FString USCClientSocket::decodeBase64(FString encodedString)
{
	FString decodedString;
	FBase64::Decode(encodedString, decodedString);
	return decodedString;
}

FString USCClientSocket::encodeBase64(FString decodedString)
{
	FString encodedString = FBase64::Encode(decodedString);
	return encodedString;
}

TSharedPtr<FJsonValue> USCClientSocket::_extractAuthTokenData(FString token)
{
	TArray<FString> tokenParts;
	token.ParseIntoArray(tokenParts, TEXT("."), true);

	FString encodedTokenData = tokenParts[1];
	if (!encodedTokenData.IsEmpty())
	{
		TSharedPtr<FJsonValue> tokenData = USCJsonConvert::ToJsonValue(decodeBase64(encodedTokenData));
		return tokenData;
	}
	return nullptr;
}

USCJsonValue* USCClientSocket::getAuthTokenBlueprint()
{
	USCJsonValue* Value = NewObject<USCJsonValue>();
	Value->SetRootValue(authToken);
	return Value;
}

TSharedPtr<FJsonValue> USCClientSocket::getAuthToken()
{
	return authToken;
}

FString USCClientSocket::getSignedAuthToken()
{
	return signedAuthToken;
}

void USCClientSocket::authenticateBlueprint(const FString& token, const FString& callback, UObject* callbackTarget)
{
	if (!callback.IsEmpty())
	{
		authenticate(token, [&, callback, callbackTarget](TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
		{
			authenticateBlueprintCallback(callback, callbackTarget, error, data);
		});
	}
	else
	{
		authenticate(token);
	}
}

void USCClientSocket::authenticateBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
{
	if (!target->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("Authenticate target not found for callback function '" + callback + "'");
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("Authenticate callback function '" + callback + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();

	if(FunctionParams >= 2)
	{

		struct FDynamicArgs
		{
			USCJsonValue* Arg01 = nullptr;
			USCJsonValue* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*") && SecondParam.Equals("USCJsonValue*"))
		{
			USCJsonValue* err = NewObject<USCJsonValue>();
			err->SetRootValue(error);

			USCJsonValue* authStatus = NewObject<USCJsonValue>();
			authStatus->SetRootValue(data);

			Args.Arg01 = err;
			Args.Arg02 = authStatus;

			target->ProcessEvent(Function, &Args);
		}
		else
		{
			USCErrors::InvalidArgumentsError("Authenticate callback function '" + callback + "' parameters incorrect");
		}
	}
	else
	{
		USCErrors::InvalidArgumentsError("Authenticate callback function '" + callback + "' parameters incorrect");
	}
}

void USCClientSocket::authenticate(FString token, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{
	TSharedPtr<FJsonObject> data = MakeShareable(new FJsonObject);
	data->SetStringField("signedAuthToken", token);
	
	emit("#authenticate", USCJsonConvert::ToJsonValue(data), [&, callback](TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data) {
		TSharedPtr<FJsonObject> authStatus = nullptr;
		if (data.IsValid() && data->Type == EJson::Object)
		{
			authStatus = data->AsObject();
			if (authStatus->HasField("isAuthenticated"))
			{
				if (authStatus->HasField("authError"))
				{
					authStatus->SetField("authError", USCErrors::Error(USCJsonConvert::ToJsonValue(authStatus->GetObjectField("authError"))));
				}
			}
			else
			{
				authStatus->SetStringField("isAuthenticated", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", authState));
				authStatus->SetObjectField("authError", nullptr);
			}
		}

		if (error.IsValid() && error->Type == EJson::Object)
		{
			TSharedPtr<FJsonObject> err = error->AsObject();
			if (!err->GetStringField("name").Equals("BadConnectionError") && !err->GetStringField("name").Equals("TimeoutError"))
			{
				_changeToUnauthenticatedStateAndClearTokens();
			}
			if (callback)
			{
				callback(error, USCJsonConvert::ToJsonValue(authStatus));
			}
		}
		else
		{
			auth->saveToken(authTokenName, token, [&, callback, authStatus](TSharedPtr<FJsonValue> err, FString token)
			{
				if (err.IsValid())
				{
					_onSCError(err);
				}
				if (authStatus->GetBoolField("isAuthenticated"))
				{
					_changeToAuthenticatedState(token);
				}
				else
				{
					_changeToUnauthenticatedStateAndClearTokens();
				}
				if (callback)
				{
					callback(err, USCJsonConvert::ToJsonValue(authStatus));
				}
			});
		}
		
	});
}

void USCClientSocket::_tryReconnect(float initialDelay)
{
	int32 exponent = connectAttempts++;
	TSharedPtr<FJsonObject> reconnectOptions = options->GetObjectField("autoReconnectOptions");
	float timeout;

	if (initialDelay == NULL || exponent > 0)
	{
		float initialTimeout = FMath::RoundToFloat(reconnectOptions->GetNumberField("initialDelay") + (reconnectOptions->GetNumberField("randomness") || 0) * FMath::RandRange(0, 1));
		timeout = FMath::RoundToFloat(initialTimeout * FMath::Pow(reconnectOptions->GetNumberField("multiplier"), exponent));
	}
	else
	{
		timeout = initialDelay;
	}

	if (timeout > reconnectOptions->GetNumberField("maxDelay"))
	{
		timeout = reconnectOptions->GetNumberField("maxDelay");
	}

	clearTimeout(_reconnectTimeoutHandle);

	pendingReconnect = true;
	pendingReconnectTimeout = timeout;

	_reconnectTimeoutRef.BindLambda([&]()
	{
		connect();
	});
	GetWorld()->GetTimerManager().SetTimer(_reconnectTimeoutHandle, _reconnectTimeoutRef, timeout, false);
}

void USCClientSocket::_onSCOpen(TSharedPtr<FJsonValue> status)
{
	preparingPendingSubscriptions = true;
	if (status.IsValid() && status->Type == EJson::Object)
	{
		TSharedPtr<FJsonObject> statusObj = status->AsObject();
;		id = statusObj->GetStringField("id");
		pingTimeout = (statusObj->GetNumberField("pingTimeout") / 1000);
		transport->pingTimeout = pingTimeout;
		if (statusObj->GetBoolField("isAuthenticated"))
		{
			_changeToAuthenticatedState(statusObj->GetStringField("authToken"));
		}
		else
		{
			_changeToUnauthenticatedStateAndClearTokens();
		}
	}
	else
	{
		_changeToUnauthenticatedStateAndClearTokens();
	}

	connectAttempts = 0;

	if (options->GetBoolField("autoSubscribeOnConnect"))
	{
		processPendingSubscriptions();
	}
	
	if (Emitter.Contains("connect") && Emitter.FindRef("connect"))
	{
		Emitter.FindRef("connect")(status, nullptr);
	}

	if (state == ESocketClusterState::OPEN)
	{
		_flushEmitBuffer();
	}
}

void USCClientSocket::_onSCError(TSharedPtr<FJsonValue> err)
{
	if (Emitter.Contains("error") && Emitter.FindRef("error"))
	{
		Emitter.FindRef("error")(err, nullptr);
	}
}

void USCClientSocket::_suspendSubscriptions()
{
	USCChannel* channel;
	ESocketClusterChannelState newState;
	for (auto& channelName : channels)
	{
		channel = channelName.Value;
		if (channel->channel_state == ESocketClusterChannelState::SUBSCRIBED)
		{
			channel->channel_state = ESocketClusterChannelState::PENDING;
			newState = ESocketClusterChannelState::PENDING;
		}
		else
		{
			newState = ESocketClusterChannelState::UNSUBSCRIBED;
		}

		_triggerChannelUnsubscribe(channel, newState);
	}
}

void USCClientSocket::_abortAllPendingEventsDueToBadConnection(FString failureType)
{
	TArray<USCEventObject*> currentNode = _emitBuffer;
	for (auto& eventObject : currentNode)
	{
		clearTimeout(eventObject->timeoutHandle);
		TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = eventObject->callback;
		if (callback)
		{
			FString errorMessage = "Event '" + eventObject->event + "' was aborted due to a bad connection";
			TSharedPtr<FJsonValue> error = USCErrors::BadConnectionError(errorMessage, failureType);
			callback(error, nullptr);
		}

		if (eventObject->cid != 0)
		{
			transport->cancelPendingResponse(eventObject->cid);
		}
	}
}

void USCClientSocket::_onSCClose(int32 code, FString data, bool openAbort)
{
	id.Empty();

	if (transport->IsValidLowLevel())
	{
		transport->off();
	}
	pendingReconnect = false;
	pendingReconnectTimeout = 0.0f;
	clearTimeout(_reconnectTimeoutHandle);

	_suspendSubscriptions();
	_abortAllPendingEventsDueToBadConnection(openAbort ? "connectAbort" : "disconnect");

	if (options->GetBoolField("autoReconnect"))
	{
		if (code == 4000 || code == 4001 || code == 1005)
		{
			_tryReconnect(0);
		}
		else if (code != 1000 && code < 4500)
		{
			_tryReconnect();
		}
	}

	if (openAbort)
	{
		if (Emitter.Contains("connectAbort") && Emitter.FindRef("connectAbort"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetNumberField("code", code);
			dataObj->SetStringField("data", data);
			Emitter.FindRef("connectAbort")(USCJsonConvert::ToJsonValue(dataObj), nullptr);
		}
	}
	else
	{
		if (Emitter.Contains("disconnect") && Emitter.FindRef("disconnect"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetNumberField("code", code);
			dataObj->SetStringField("data", data);
			Emitter.FindRef("disconnect")(USCJsonConvert::ToJsonValue(dataObj), nullptr);
		}
	}

	if (Emitter.Contains("close") && Emitter.FindRef("close"))
	{
		TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
		dataObj->SetNumberField("code", code);
		dataObj->SetStringField("data", data);
		Emitter.FindRef("close")(USCJsonConvert::ToJsonValue(dataObj), nullptr);
	}

	if (!USCErrors::socketProtocolIgnoreStatuses.Contains(code))
	{
		FString closeMessage;
		if (!data.IsEmpty())
		{
			closeMessage = "Socket connection closed with status code " + FString::FromInt(code) + " and reason: " + data;
		}
		else
		{
			closeMessage = "Socket connection closed with status code " + FString::FromInt(code);
		}
		TSharedPtr<FJsonValue> err = USCErrors::SocketProtocolError(USCErrors::socketProtocolErrorStatuses.Contains(code) ? USCErrors::socketProtocolErrorStatuses[code] : closeMessage, code);
		_onSCError(err);
	}
}

void USCClientSocket::_onSCEvent(FString event, TSharedPtr<FJsonValue> data, USCResponse* res)
{
	if (_privateEventHandlerMap.Contains(event))
	{
		TFunction<void(TSharedPtr<FJsonValue>, USCResponse*)> handler = _privateEventHandlerMap[event];
		if (handler)
		{
			handler(data, res);
		}
	}
	else
	{
		if (Emitter.Contains(event) && Emitter.FindRef(event))
		{
			Emitter.FindRef(event)(data, res);
		}
	}
}

TSharedPtr<FJsonValue> USCClientSocket::decode(FString message)
{
	return transport->decode(message);
}

FString USCClientSocket::encode(TSharedPtr<FJsonValue> object)
{
	return transport->encode(object);
}

void USCClientSocket::_flushEmitBuffer()
{
	TArray<USCEventObject*> currentNodes = _emitBuffer;
	for (auto& eventObject : currentNodes)
	{
		transport->emitObject(eventObject);
		_emitBuffer.Remove(eventObject);
	}
}

void USCClientSocket::_handleEventAckTimeout(USCEventObject* eventObject)
{
	clearTimeout(eventObject->timeoutHandle);

	TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback = eventObject->callback;
	if (callback)
	{
		eventObject->callback = nullptr;
		TSharedPtr<FJsonValue> error = USCErrors::TimeoutError("Event response for '" + eventObject->event + "' timed out");
		callback(error, nullptr);
	}

	if (eventObject->cid != 0)
	{
		transport->cancelPendingResponse(eventObject->cid);
	}
}

void USCClientSocket::_emit(FString event, TSharedPtr<FJsonValue> data, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{

	if (state == ESocketClusterState::CLOSED)
	{
		connect();
	}

	USCEventObject* eventObject = NewObject<USCEventObject>();
	eventObject->event = event;
	eventObject->callback = callback;
	eventObject->data = data;
	eventObject->timeout = FTimerDelegate::CreateUObject(this, &USCClientSocket::_handleEventAckTimeout, eventObject);
	GetWorld()->GetTimerManager().SetTimer(eventObject->timeoutHandle, eventObject->timeout, options->GetNumberField("ackTimeout"), false);

	_emitBuffer.Add(eventObject);
	if (state == ESocketClusterState::OPEN)
	{
		_flushEmitBuffer();
	}
}

void USCClientSocket::send(const FString& data)
{
	if (transport->IsValidLowLevel())
	{
		transport->send(data);
	}
}

void USCClientSocket::emitBlueprint(const FString& event, USCJsonValue* data, const FString& callback, UObject* callbackTarget)
{
	TSharedPtr<FJsonValue> DataValue = nullptr;
	if (data != nullptr)
	{
		DataValue = data->GetRootValue();
	}
	else
	{
		DataValue = MakeShareable(new FJsonValueNull);
	}

	if (!callback.IsEmpty())
	{
		
		emit(event, DataValue, [&, callback, callbackTarget](TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
		{
			emitBlueprintCallback(callback, callbackTarget, error, data);
		});
	}
	else
	{
		emit(event, DataValue);
	}
}

void USCClientSocket::emitBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
{
	if (!target->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("Emit target not found for callback function '" + callback + "'");
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("Emit callback function '" + callback + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();

	if(FunctionParams >= 2)
	{

		struct FDynamicArgs
		{
			USCJsonValue* Arg01 = nullptr;
			USCJsonValue* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*") && SecondParam.Equals("USCJsonValue*"))
		{
			USCJsonValue* ErrorValue = NewObject<USCJsonValue>();
			ErrorValue->SetRootValue(error);

			USCJsonValue* DataValue = NewObject<USCJsonValue>();
			DataValue->SetRootValue(data);

			Args.Arg01 = ErrorValue;
			Args.Arg02 = DataValue;
			target->ProcessEvent(Function, &Args);
		}
		else
		{
			USCErrors::InvalidArgumentsError("Emit callback function '" + callback + "' parameters incorrect");
		}
	}
	else
	{
		USCErrors::InvalidArgumentsError("Emit callback function '" + callback + "' parameters incorrect");
	}
}

void USCClientSocket::emit(FString event, TSharedPtr<FJsonValue> data, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{
	if (!_localEvents.Contains(event))
	{
		_emit(event, data, callback);
	}
	else if (event.Equals("error"))
	{
		if (Emitter.Contains(event) && Emitter.FindRef(event))
		{
			Emitter.FindRef(event)(data, nullptr);
		}
	}
	else
	{
		TSharedPtr<FJsonValue> error = USCErrors::InvalidActionError("The '" + event + "' event is reserved and cannot be emitted on a client socket");
		_onSCError(error);
	}
}

void USCClientSocket::onBlueprint(const FString& event, const FString& handler, UObject* handlerTarget)
{
	if (!handler.IsEmpty())
	{
		on(event, [&, event, handler, handlerTarget](TSharedPtr<FJsonValue> data, USCResponse* res) {
			onBlueprintHandler(event, handler, handlerTarget, data, res);
		});
	}
}

void USCClientSocket::onBlueprintHandler(const FString& event, const FString& handler, UObject* handlerTarget, TSharedPtr<FJsonValue> data, USCResponse* res)
{
	if (!handlerTarget->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("On target not found for handler function '" + handler + "'");
		return;
	}

	UFunction* Function = handlerTarget->FindFunction(FName(*handler));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("On handler function '" + handler + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();
	
	if (FunctionParams == 1)
	{

		struct FDynamicArgs
		{
			USCJsonValue* Arg01 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*"))
		{
			USCJsonValue* Value = NewObject<USCJsonValue>();
			Value->SetRootValue(data);
			Args.Arg01 = Value;
			handlerTarget->ProcessEvent(Function, &Args);
		}
		else
		{
			USCErrors::InvalidArgumentsError("On handler function '" + handler + "' parameters incorrect");
			return;
		}
	}
	else if (FunctionParams >= 2)
	{
		struct FDynamicArgs
		{
			USCJsonValue* Arg01 = nullptr;
			USCResponse* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();
		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*") && SecondParam.Equals("USCResponse*"))
		{
			if (res != nullptr)
			{
				USCJsonValue* Value = NewObject<USCJsonValue>();
				Value->SetRootValue(data);
				Args.Arg01 = Value;
				Args.Arg02 = res;
				handlerTarget->ProcessEvent(Function, &Args);
			}
			else
			{
				USCErrors::InvalidArgumentsError("Event '" + event + "' does not support a Response object");
			}
		}
		else
		{
			USCErrors::InvalidArgumentsError("On handler function '" + handler + "' parameters incorrect");
		}	
	}
}

void USCClientSocket::on(FString event, TFunction<void(TSharedPtr<FJsonValue>, USCResponse*)> handler)
{
	Emitter.Add(event, handler);
}

void USCClientSocket::off(FString event)
{
	Emitter.Remove(event);
}

void USCClientSocket::publishBlueprint(const FString& channelName, USCJsonValue* data, const FString& callback, UObject* callbackTarget)
{
	TSharedPtr<FJsonValue> DataValue = nullptr;
	if (data != nullptr)
	{
		DataValue = data->GetRootValue();
	}
	else
	{
		DataValue = MakeShareable(new FJsonValueNull);
	}

	if (!callback.IsEmpty())
	{
		publish(channelName, DataValue, [&, callback, callbackTarget, this](TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
		{
			publishBlueprintCallback(callback, callbackTarget, error, data);
		});
	}
	else
	{
		publish(channelName, DataValue);
	}
}

void USCClientSocket::publishBlueprintCallback(const FString& callback, UObject* target, TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
{
	if (!target->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("Publish target not found for callback function '" + callback + "'");
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("Publish callback function '" + callback + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();

	if(FunctionParams >= 2)
	{

		struct FDynamicArgs
		{
			USCJsonValue* Arg01 = nullptr;
			USCJsonValue* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*") && SecondParam.Equals("USCJsonValue*"))
		{
			USCJsonValue* ErrorValue = NewObject<USCJsonValue>();
			ErrorValue->SetRootValue(error);
			Args.Arg01 = ErrorValue;

			USCJsonValue* DataValue = NewObject<USCJsonValue>();
			DataValue->SetRootValue(data);
			Args.Arg02 = DataValue;

			target->ProcessEvent(Function, &Args);
		}
		else
		{
			USCErrors::InvalidArgumentsError("Publish callback function '" + callback + "' parameters incorrect");
		}
	}
	else
	{
		USCErrors::InvalidArgumentsError("Publish callback function '" + callback + "' parameters incorrect");
	}
}

void USCClientSocket::publish(FString channelName, TSharedPtr<FJsonValue> data, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{
	TSharedPtr<FJsonObject> pubData = MakeShareable(new FJsonObject);
	pubData->SetStringField("channel", _decorateChannelName(channelName));
	pubData->SetField("data", data);
	emit("#publish", USCJsonConvert::ToJsonValue(pubData), callback);
}

void USCClientSocket::_triggerChannelSubscribe(USCChannel* channel, TSharedPtr<FJsonObject> subscriptionOptions)
{
	FString channelName = channel->channel_name;

	if (channel->channel_state != ESocketClusterChannelState::SUBSCRIBED)
	{
		ESocketClusterChannelState oldState = channel->channel_state;
		channel->channel_state = ESocketClusterChannelState::SUBSCRIBED;

		TSharedPtr<FJsonObject> stateChangeData = MakeShareable(new FJsonObject);
		stateChangeData->SetStringField("channel", channelName);
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", channel->channel_state));

		if (channel->Emitter.Contains("subscribeStateChange") && channel->Emitter.FindRef("subscribeStateChange"))
		{
			channel->Emitter.FindRef("subscribeStateChange")(USCJsonConvert::ToJsonValue(stateChangeData));
		}
		if (channel->Emitter.Contains("subscribe") && channel->Emitter.FindRef("subscribe"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetStringField("channelName", channelName);
			dataObj->SetObjectField("subscriptionOptions", subscriptionOptions);
			channel->Emitter.FindRef("subscribe")(USCJsonConvert::ToJsonValue(dataObj));
		}

		if (Emitter.Contains("subscribeStateChange") && Emitter.FindRef("subscribeStateChange"))
		{
			Emitter.FindRef("subscribeStateChange")(USCJsonConvert::ToJsonValue(stateChangeData), nullptr);
		}
		if (Emitter.Contains("subscribe") && Emitter.FindRef("subscribe"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetStringField("channelName", channelName);
			dataObj->SetObjectField("subscriptionOptions", subscriptionOptions);
			Emitter.FindRef("subscribe")(USCJsonConvert::ToJsonValue(dataObj), nullptr);
		}
	}
}

void USCClientSocket::_triggerChannelSubscribeFail(TSharedPtr<FJsonValue> err, USCChannel* channel, TSharedPtr<FJsonObject> subscriptionOptions)
{
	FString channelName = channel->channel_name;
	bool meetsAuthRequirements = !channel->channel_waitForAuth || authState == ESocketClusterAuthState::AUTHENTICATED;

	if (channel->channel_state != ESocketClusterChannelState::UNSUBSCRIBED && meetsAuthRequirements)
	{
		channel->channel_state = ESocketClusterChannelState::UNSUBSCRIBED;

		if (channel->Emitter.Contains("subscribeFail") && channel->Emitter.FindRef("subscribeFail"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetField("error", err);
			dataObj->SetStringField("channelName", channelName);
			dataObj->SetObjectField("subscriptionOptions", subscriptionOptions);
			channel->Emitter.FindRef("subscribe")(USCJsonConvert::ToJsonValue(dataObj));
		}
		if (Emitter.Contains("subscribeFail") && Emitter.FindRef("subscribeFail"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetField("error", err);
			dataObj->SetStringField("channelName", channelName);
			dataObj->SetObjectField("subscriptionOptions", subscriptionOptions);
			Emitter.FindRef("subscribe")(USCJsonConvert::ToJsonValue(dataObj), nullptr);
		}
	}
}

void USCClientSocket::_cancelPendingSubscribeCallback(USCChannel* channel)
{
	if (channel->channel_pendingSubscriptionCid != 0)
	{
		transport->cancelPendingResponse(channel->channel_pendingSubscriptionCid);
		channel->channel_pendingSubscriptionCid = 0;
	}
}

FString USCClientSocket::_decorateChannelName(FString channelName)
{
	if (!channelPrefix.IsEmpty())
	{
		channelName = channelPrefix.Append(channelName);
	}
	return channelName;
}

FString USCClientSocket::_undecorateChannelName(FString decoratedChannelName)
{
	if (!channelPrefix.IsEmpty() && decoratedChannelName.StartsWith(channelPrefix))
	{
		decoratedChannelName.RemoveFromStart(channelPrefix);
	}
	return decoratedChannelName;
}

void USCClientSocket::_trySubscribe(USCChannel* channel)
{

	bool meetsAuthRequirements = !channel->channel_waitForAuth || authState == ESocketClusterAuthState::AUTHENTICATED;

	if (state == ESocketClusterState::OPEN && !preparingPendingSubscriptions && channel->channel_pendingSubscriptionCid == 0 && meetsAuthRequirements)
	{

		TSharedPtr<FJsonObject> opts = MakeShareable(new FJsonObject);
		opts->SetBoolField("noTimeout", true);

		TSharedPtr<FJsonObject> subscriptionOptions = MakeShareable(new FJsonObject);
		subscriptionOptions->SetStringField("channel", _decorateChannelName(channel->channel_name));

		if (channel->channel_waitForAuth)
		{
			opts->SetBoolField("waitForAuth", true);
			subscriptionOptions->SetBoolField("waitForAuth", true);
		}

		if (channel->channel_data)
		{
			subscriptionOptions->SetField("data", channel->channel_data);
		}

		if (channel->channel_batch)
		{
			opts->SetBoolField("batch", true);
			subscriptionOptions->SetBoolField("batch", true);
		}

		channel->channel_pendingSubscriptionCid = transport->emit("#subscribe", USCJsonConvert::ToJsonValue(subscriptionOptions), opts, [&, channel, subscriptionOptions](TSharedPtr<FJsonValue> err, TSharedPtr<FJsonValue> data)
		{
			channel->channel_pendingSubscriptionCid = 0;
			if (err.IsValid())
			{
				_triggerChannelSubscribeFail(err, channel, subscriptionOptions);
			}
			else
			{
				_triggerChannelSubscribe(channel, subscriptionOptions);
			}
		});
		if (Emitter.Contains("subscribeRequest") && Emitter.FindRef("subscribeRequest"))
		{
			TSharedPtr<FJsonObject> dataObj = MakeShareable(new FJsonObject);
			dataObj->SetStringField("channelName", channel->channel_name);
			dataObj->SetObjectField("subscriptionOptions", subscriptionOptions);
			Emitter.FindRef("subscribeRequest")(USCJsonConvert::ToJsonValue(dataObj), nullptr);
		}
	}
}

USCChannel* USCClientSocket::subscribeBlueprint(const FString& channelName, const bool waitForAuth, USCJsonValue* data, const bool batch)
{
	TSharedPtr<FJsonObject> opts = MakeShareable(new FJsonObject);
	opts->SetBoolField("waitForAuth", waitForAuth);

	TSharedPtr<FJsonValue> dataValue;
	if (data != nullptr)
	{
		dataValue = data->GetRootValue();
		opts->SetField("data", dataValue);
	}

	
	opts->SetBoolField("batch", batch);

	return subscribe(channelName, opts);
}

USCChannel* USCClientSocket::subscribe(const FString& channelName, TSharedPtr<FJsonObject> opts)
{
	USCChannel* channel = channels.FindRef(channelName);
	if (!channel)
	{
		channel = USCChannel::create(channelName, this, opts);
		channels.Add(channelName, channel);
	}
	else
	{
		channel->setOptions(opts);
	}

	if (channel->channel_state == ESocketClusterChannelState::UNSUBSCRIBED)
	{
		channel->channel_state = ESocketClusterChannelState::PENDING;
		_trySubscribe(channel);
	}
	return channel;
}

void USCClientSocket::_triggerChannelUnsubscribe(USCChannel* channel, ESocketClusterChannelState newState)
{
	FString channelName = channel->channel_name;
	ESocketClusterChannelState oldState = channel->channel_state;

	channel->channel_state = newState;

	_cancelPendingSubscribeCallback(channel);

	if (oldState == ESocketClusterChannelState::SUBSCRIBED)
	{
		TSharedPtr<FJsonObject> stateChangeData = MakeShareable(new FJsonObject);
		stateChangeData->SetStringField("channel", channelName);
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", channel->channel_state));

		if (channel->Emitter.Contains("subscribeStateChange") && channel->Emitter.FindRef("subscribeStateChange"))
		{
			channel->Emitter.FindRef("subscribeStateChange")(USCJsonConvert::ToJsonValue(stateChangeData));
		}
		if (channel->Emitter.Contains("unsubscribe") && channel->Emitter.FindRef("unsubscribe"))
		{
			channel->Emitter.FindRef("unsubscribe")(USCJsonConvert::ToJsonValue(channelName));
		}
		if (Emitter.Contains("subscribeStateChange") && Emitter.FindRef("subscribeStateChange"))
		{
			Emitter.FindRef("subscribeStateChange")(USCJsonConvert::ToJsonValue(stateChangeData), nullptr);
		}
		if (Emitter.Contains("unsubscribe") && Emitter.FindRef("unsubscribe"))
		{
			Emitter.FindRef("unsubscribe")(USCJsonConvert::ToJsonValue(channelName), nullptr);
		}
	}
}

void USCClientSocket::_tryUnsubscribe(USCChannel* channel)
{
	if (state == ESocketClusterState::OPEN)
	{
		TSharedPtr<FJsonObject> opts = MakeShareable(new FJsonObject);
		opts->SetBoolField("noTimeout", true);
		if (channel->channel_batch)
		{
			opts->SetBoolField("batch", true);
		}
		_cancelPendingSubscribeCallback(channel);

		FString decoratedChannelName = _decorateChannelName(channel->channel_name);
		TSharedPtr<FJsonValue> data = MakeShareable(new FJsonValueString(decoratedChannelName));
		transport->emit("#unsubscribe", data, opts);
	}
}

void USCClientSocket::unsubscribe(const FString& channelName)
{
	USCChannel* channel = channels.FindRef(channelName);

	if (channel)
	{
		if (channel->channel_state != ESocketClusterChannelState::UNSUBSCRIBED)
		{
			_triggerChannelUnsubscribe(channel);
			_tryUnsubscribe(channel);
		}
	}
}

USCChannel* USCClientSocket::channelBlueprint(const FString& channelName, const bool waitForAuth, USCJsonValue* data, const bool batch)
{
	TSharedPtr<FJsonObject> opts = MakeShareable(new FJsonObject);
	opts->SetBoolField("waitForAuth", waitForAuth);

	TSharedPtr<FJsonValue> dataValue = nullptr;
	if (data != nullptr)
	{
		dataValue = data->GetRootValue();
	}
	else
	{
		dataValue = MakeShareable(new FJsonValueNull);
	}
	opts->SetField("data", dataValue);
	opts->SetBoolField("batch", batch);

	return channel(channelName, opts);
}

USCChannel* USCClientSocket::channel(const FString& channelName, TSharedPtr<FJsonObject> opts)
{
	USCChannel* currentChannel = channels.FindRef(channelName);

	if (!currentChannel)
	{
		currentChannel = USCChannel::create(channelName, this, opts);
		channels.Add(channelName, currentChannel);
	}
	return currentChannel;
}

void USCClientSocket::destroyChannel(const FString& channelName)
{
	USCChannel* channel = channels.FindRef(channelName);

	if (channel)
	{
		channel->unwatch();
		channel->unsubscribe();
		channels.Remove(channelName);
	}
}

TArray<FString> USCClientSocket::subscriptions(const bool includePending)
{
	TArray<FString> subs = TArray<FString>();
	for (auto& channel : channels)
	{
		bool includeChannel;
		FString channelName = channel.Key;
		if (includePending)
		{
			includeChannel = (channel.Value->channel_state == ESocketClusterChannelState::SUBSCRIBED || channel.Value->channel_state == ESocketClusterChannelState::PENDING);
		}
		else
		{
			includeChannel = channel.Value->channel_state == ESocketClusterChannelState::SUBSCRIBED;
		}

		if (includeChannel)
		{
			subs.Add(channelName);
		}
	}
	return subs;
}

bool USCClientSocket::isSubscribed(const FString& channelName, const bool includePending)
{
	USCChannel* channel = channels.FindRef(channelName);
	if (includePending)
	{
		return !!channel && (channel->channel_state == ESocketClusterChannelState::SUBSCRIBED || channel->channel_state == ESocketClusterChannelState::PENDING);
	}
	return !!channel && channel->channel_state == ESocketClusterChannelState::SUBSCRIBED;
}

void USCClientSocket::processPendingSubscriptions()
{

	preparingPendingSubscriptions = false;

	TArray<USCChannel*> pendingChannels = TArray<USCChannel*>();
	for (auto& channel : channels)
	{
		if(channel.Value->channel_state == ESocketClusterChannelState::PENDING)
		{
			pendingChannels.Add(channel.Value);
		}
	}

	for (auto& pendingChannel : pendingChannels)
	{
		_trySubscribe(pendingChannel);
	}
}

void USCClientSocket::watchBlueprint(const FString& channelName, const FString& handler, UObject* handlerTarget)
{
	if (!handler.IsEmpty())
	{
		watch(channelName, [&, handler, handlerTarget](TSharedPtr<FJsonValue> data)
		{
			watchBlueprintCallback(handler, handlerTarget, data);
		});
	}
	else
	{
		USCErrors::InvalidArgumentsError("No handler function was provided");
	}
}

void USCClientSocket::watchBlueprintCallback(const FString& handler, UObject* target, TSharedPtr<FJsonValue> data)
{
	if (!target->IsValidLowLevel())
	{
		USCErrors::InvalidActionError("Watch target not found for handler function '" + handler + "'");
		return;
	}

	UFunction* Function = target->FindFunction(FName(*handler));
	if (nullptr == Function)
	{
		USCErrors::InvalidActionError("Watch handler function '" + handler + "' not found");
		return;
	}

	TFieldIterator<UProperty> Iterator(Function);

	TArray<UProperty*> Properties;
	while (Iterator && (Iterator->PropertyFlags & CPF_Parm))
	{
		UProperty* Prop = *Iterator;
		Properties.Add(Prop);
		++Iterator;
	}

	int32 FunctionParams = Properties.Num();

	if (FunctionParams >= 1)
	{

		struct FDynamicArgs
		{
			void* Arg01 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();

		if (FirstParam.Equals("USCJsonValue*"))
		{
			USCJsonValue* DataValue = NewObject<USCJsonValue>();
			DataValue->SetRootValue(data);
			Args.Arg01 = DataValue;
			target->ProcessEvent(Function, &Args);
			
		}
		else if (FirstParam.Equals("FString"))
		{
			FString	StringValue = USCJsonConvert::ToJsonString(data);
			target->ProcessEvent(Function, &StringValue);
		}
		else
		{
			USCErrors::InvalidArgumentsError("Watch handler function '" + handler + "' parameters incorrect");
		}
	}
	else
	{
		USCErrors::InvalidArgumentsError("Watch handler function '" + handler + "' parameters incorrect");
	}
}

void USCClientSocket::watch(FString channelName, TFunction<void(TSharedPtr<FJsonValue>)> handler)
{
	if (!handler)
	{
		USCErrors::InvalidArgumentsError("No handler function was provided");
		return;
	}
	_channelEmitter.Add(channelName, handler);
}

void USCClientSocket::unwatch(const FString& channelName)
{
	_channelEmitter.Remove(channelName);
}

TArray<TFunction<void(TSharedPtr<FJsonValue>)>> USCClientSocket::watchers(FString channelName)
{
	TArray<TFunction<void(TSharedPtr<FJsonValue>)>> watchers;
	_channelEmitter.MultiFind(channelName, watchers);
	return watchers;
}

void USCClientSocket::clearTimeout(FTimerHandle timer)
{
	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(this, timer))
	{
		GetWorld()->GetTimerManager().ClearTimer(timer);
	}
}

FString USCClientSocket::queryParse(TSharedPtr<FJsonObject> query)
{
	FString queryString;
	if (query.IsValid() && query->Values.Num() > 0)
	{
		for (auto Pair : query->Values)
		{
			TSharedPtr<FJsonValue> Value = Pair.Value;
			if (Value->Type == EJson::String)
			{
				if (queryString.IsEmpty())
				{
					queryString.Append(TEXT("?")).Append(Pair.Key).Append(TEXT("=")).Append(Value->AsString());
				}
				else
				{
					queryString.Append(TEXT("&")).Append(Pair.Key).Append(TEXT("=")).Append(Value->AsString());
				}
			}
		}
	}
	return queryString;
}