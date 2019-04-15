// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCClientSocket.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
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

	_privateEventHandlerMap.Add("#publish", [&](USCJsonObject* data, USCResponse* response)
	{
		FString undecoratedChannelName = _undecorateChannelName(data->GetStringField("channel"));
		bool IsSubscribed = isSubscribed(undecoratedChannelName, true);
		if (IsSubscribed)
		{
			TArray<TFunction<void(USCJsonObject*)>> listeners;
			_channelEmitter.MultiFind(undecoratedChannelName, listeners);
			for (auto& listener : listeners)
			{
				listener(data->GetObjectField("data"));
			}

		}
	});
	_privateEventHandlerMap.Add("#kickOut", [&](USCJsonObject* data, USCResponse* response)
	{
		FString undecoratedChannelName = _undecorateChannelName(data->GetStringField("channel"));
		USCChannel* channel = channels.FindRef(undecoratedChannelName);
		if (channel)
		{
			OnKickOut.Broadcast(data->GetStringField("message"), undecoratedChannelName);
			channel->OnChannelKickOut.Broadcast(data->GetStringField("message"), undecoratedChannelName);
			_triggerChannelUnsubscribe(channel);
		}
	});
	_privateEventHandlerMap.Add("#setAuthToken", [&](USCJsonObject* data, USCResponse* response)
	{
		if (data)
		{
			auth->saveToken(authTokenName, data->GetStringField("token"), [&](USCJsonObject* err, FString token) 
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
	_privateEventHandlerMap.Add("#removeAuthToken", [&](USCJsonObject* data, USCResponse* response)
	{
		auth->removeToken(authTokenName, [&](USCJsonObject* err, FString oldToken)
		{
			if (err)
			{
				response->error(err);
				_onSCError(err);
			}
			else
			{
				OnRemoveAuthToken.Broadcast(oldToken);
				_changeToUnauthenticatedStateAndClearTokens();
				response->end();
			}
		});
	});
	_privateEventHandlerMap.Add("#disconnect", [&](USCJsonObject* data, USCResponse* response)
	{
		transport->close(data->GetIntegerField("code"), data->GetObjectField("data"));
	});
}

void USCClientSocket::BeginDestroy()
{
	Emitter.Empty();
	destroy();
	Super::BeginDestroy();
}

UWorld* USCClientSocket::GetWorld() const
{
	return World;
}

void USCClientSocket::create(const UObject* WorldContextObject, TSubclassOf<USCAuthEngine> authEngine, TSubclassOf<USCCodecEngine> codecEngine, USCJsonObject* opts)
{

	World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Unable to access current game world."), *SCC_FUNC_LINE);
		return;
	}

	id.Empty();
	state = ESocketClusterState::CLOSED;
	authState = ESocketClusterAuthState::UNAUTHENTICATED;
	signedAuthToken.Empty();
	authToken = nullptr;
	pendingReconnect = false;
	pendingReconnectTimeout = 0.0f;
	preparingPendingSubscriptions = false;
	clientId = opts->GetStringField("clientId");

	connectTimeout = opts->GetNumberField("connectTimeout");
	ackTimeout = opts->GetNumberField("ackTimeout");
	channelPrefix = opts->GetStringField("channelPrefix");
	authTokenName = opts->GetStringField("authTokenName");

	pingTimeout = this->ackTimeout;
	pingTimeoutDisabled = opts->GetBoolField("pingTimeoutDisabled");
	active = true;

	connectAttempts = 0;

	_emitBuffer.Empty();
	channels.Empty();

	options = opts;

	_cid = 1;

	options->SetIntegerField("callIdGenerator", _cid);

	if (options->GetBoolField("autoReconnect"))
	{
		USCJsonObject* reconnectOptions = options->GetObjectField("autoReconnectOptions");
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
		this->codec = codecEngine->GetDefaultObject<USCCodecEngine>();
	}
	else
	{
		this->codec = NewObject<USC_Formatter>();
	}

	options->SetStringField("query", queryParse(opts->GetObjectArrayField("query")));

	if (options->GetBoolField("autoConnect"))
	{
		connect();
	}
}

ESocketClusterState USCClientSocket::getState()
{
	return state;
}

void USCClientSocket::deauthenticateBlueprint(const FString& Callback, UObject* CallbackTarget)
{
	if (!Callback.IsEmpty())
	{
		deauthenticate([&, Callback, CallbackTarget, this](USCJsonObject* error)
		{
			deauthenticateBlueprintCallback(CallbackTarget, Callback, error);
		});
	}
	else
	{
		deauthenticate();
	}
}

void USCClientSocket::deauthenticateBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error)
{
	if (!target->IsValidLowLevel())
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Deauthenticate target not found for callback function : %s"), *SCC_FUNC_LINE, *callback);
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Deauthenticate callback function '%s' not found"), *SCC_FUNC_LINE, *callback);
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

	if (FunctionParams == 0)
	{
		target->ProcessEvent(Function, nullptr);
	}
	else
	{
		const FString& FirstParam = Properties[0]->GetCPPType();
		if (FirstParam.Equals("USCJsonObject*"))
		{
			struct FDynamicArgs
			{
				USCJsonObject* Arg01 = nullptr;
			};

			FDynamicArgs Args = FDynamicArgs();
			Args.Arg01 = error;

			target->ProcessEvent(Function, &Args);
		}
		else
		{
			UE_LOG(LogSCClient, Error, TEXT("%s : Deauthenticate callback function '%s' parameters incorrect"), *SCC_FUNC_LINE, *callback);
		}

	}
}

void USCClientSocket::deauthenticate(TFunction<void(USCJsonObject*)> callback)
{
	auth->removeToken(authTokenName, [&](USCJsonObject* err, FString& oldToken)
	{
		if (err)
		{
			_onSCError(err);
		}
		else
		{
			OnRemoveAuthToken.Broadcast(oldToken);
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
		USCJsonObject* error = USCErrors::InvalidActionError("Cannot connect a destroyed client");
		_onSCError(error);
		return;
	}

	if (state == ESocketClusterState::CLOSED)
	{
		pendingReconnect = false;
		pendingReconnectTimeout = 0.0f;
		clearTimeout(_reconnectTimeoutHandle);

		state = ESocketClusterState::CONNECTING;
		OnConnecting.Broadcast();

		if (transport)
		{
			transport = nullptr;
		}

		transport = NewObject<USCTransport>(this);
		transport->create(auth, codec, options);

		transport->onopen = [&](USCJsonObject* status)
		{
			state = ESocketClusterState::OPEN;
			_onSCOpen(status);
		};

		transport->onerror = [&](USCJsonObject* err)
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

		transport->onevent = [&](FString event, USCJsonObject* data, USCResponse* res)
		{
			_onSCEvent(event, data, res);
		};

		transport->onraw = [&](FString message)
		{
			OnRaw.Broadcast(message);
		};

		transport->onmessage = [&](FString message)
		{
			OnMessage.Broadcast(message);
		};
	}
}

void USCClientSocket::reconnect(int32 code, USCJsonObject* data)
{
	disconnect(code, data);
	connect();
}

void USCClientSocket::disconnect(int32 code, USCJsonObject* data)
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

void USCClientSocket::destroy(int32 code, USCJsonObject* data)
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

		USCJsonObject* stateChangeData = NewObject<USCJsonObject>();
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", authState));

		OnAuthStateChange.Broadcast(stateChangeData);
		OnDeauthenticate.Broadcast(oldSignedToken);
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

		USCJsonObject* stateChangeData = NewObject<USCJsonObject>();
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", authState));
		stateChangeData->SetStringField("signedAuthToken", signedAuthToken);
		stateChangeData->SetObjectField("authToken", authToken);

		if (!preparingPendingSubscriptions)
		{
			processPendingSubscriptions();
		}

		OnAuthStateChange.Broadcast(stateChangeData);
	}

	OnAuthenticate.Broadcast(signedAuthToken);
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

USCJsonObject* USCClientSocket::_extractAuthTokenData(FString token)
{
	TArray<FString> tokenParts;
	token.ParseIntoArray(tokenParts, TEXT("."), true);

	FString encodedTokenData = tokenParts[1];
	if (!encodedTokenData.IsEmpty())
	{
		USCJsonObject* tokenData = NewObject<USCJsonObject>();
		tokenData->DecodeJson(decodeBase64(encodedTokenData));
		return tokenData;
	}
	return nullptr;
}

USCJsonObject* USCClientSocket::getAuthToken()
{
	return authToken;
}

FString USCClientSocket::getSignedAuthToken()
{
	return signedAuthToken;
}

void USCClientSocket::authenticateBlueprint(const FString& token, const FString& Callback, UObject* CallbackTarget)
{
	if (!Callback.IsEmpty())
	{
		authenticate(token, [&, Callback, CallbackTarget, this](USCJsonObject* error, USCJsonObject* data)
		{
			authenticateBlueprintCallback(CallbackTarget, Callback, error, data);
		});
	}
	else
	{
		authenticate(token);
	}
}

void USCClientSocket::authenticateBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error, USCJsonObject* data)
{
	if (!target->IsValidLowLevel())
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Authenticate target not found for callback function : %s"), *SCC_FUNC_LINE, *callback);
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Authenticate callback function '%s' not found"), *SCC_FUNC_LINE, *callback);
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

	if (FunctionParams == 0)
	{
		target->ProcessEvent(Function, nullptr);
	}
	else
	{

		struct FDynamicArgs
		{
			USCJsonObject* Arg01 = nullptr;
			USCJsonObject* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonObject*"))
		{
			Args.Arg01 = error;
		}
		else
		{
			UE_LOG(LogSCClient, Error, TEXT("%s : Authenticate callback function '%s' parameters incorrect"), *SCC_FUNC_LINE, *callback);
			return;
		}

		if (SecondParam.Equals("USCJsonObject*"))
		{
			Args.Arg02 = data;
		}

		target->ProcessEvent(Function, &Args);
	}
}

void USCClientSocket::authenticate(FString token, TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
{
	USCJsonObject* data = NewObject<USCJsonObject>();
	data->SetStringField("signedAuthToken", token);
	
	emit("#authenticate", data, [&](USCJsonObject* err, USCJsonObject* authStatus) {
		if (authStatus && authStatus->HasField("isAuthenticated"))
		{
			if (authStatus->HasField("authError"))
			{
				authStatus->SetObjectField("authError", USCErrors::Error(authStatus->GetObjectField("authError")));
			}
		}
		else
		{
			authStatus = NewObject<USCJsonObject>();
			authStatus->SetStringField("isAuthenticated", USCJsonConvert::EnumToString<ESocketClusterAuthState>("ESocketClusterAuthState", authState));
			authStatus->SetObjectField("authError", nullptr);
		}
		if (err)
		{
			if (!err->GetStringField("name").Equals("BadConnectionError") && !err->GetStringField("name").Equals("TimeoutError"))
			{
				_changeToUnauthenticatedStateAndClearTokens();
			}
			if (callback)
			{
				callback(err, authStatus);
			}
		}
		else
		{
			auth->saveToken(authTokenName, token, [&](USCJsonObject* err, FString token) 
			{
				if (err)
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
					callback(err, authStatus);
				}
			});
		}
		
	});
}

void USCClientSocket::_tryReconnect(float initialDelay)
{
	int32 exponent = connectAttempts++;
	USCJsonObject* reconnectOptions = options->GetObjectField("autoReconnectOptions");
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

void USCClientSocket::_onSCOpen(USCJsonObject* status)
{
	preparingPendingSubscriptions = true;

	if (status)
	{
		id = status->GetStringField("id");
		pingTimeout = (status->GetNumberField("pingTimeout") / 1000);
		transport->pingTimeout = pingTimeout;
		if (status->GetBoolField("isAuthenticated"))
		{
			_changeToAuthenticatedState(status->GetStringField("authToken"));
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

	OnConnect.Broadcast(status);

	if (state == ESocketClusterState::OPEN)
	{
		_flushEmitBuffer();
	}
}

void USCClientSocket::_onSCError(USCJsonObject* err)
{
	OnError.Broadcast(err);
}

void USCClientSocket::_suspendSubscriptions()
{
	USCChannel* channel;
	ESocketClusterChannelState newState;
	for (auto& channelName : channels)
	{
		channel = channelName.Value;
		if (channel->_state == ESocketClusterChannelState::SUBSCRIBED)
		{
			channel->_state = ESocketClusterChannelState::PENDING;
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
		TFunction<void(USCJsonObject*, USCJsonObject*)> callback = eventObject->callback;
		if (callback)
		{
			FString errorMessage = "Event '" + eventObject->event + "' was aborted due to a bad connection";
			USCJsonObject* error = USCErrors::BadConnectionError(errorMessage, failureType);
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

	if (transport)
	{
		transport = nullptr;
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
		OnConnectAbort.Broadcast(code, data);
	}
	else
	{
		OnDisconnect.Broadcast(code, data);
	}
	OnClose.Broadcast(code, data);

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
		USCJsonObject* err = USCErrors::SocketProtocolError(USCErrors::socketProtocolErrorStatuses.Contains(code) ? USCErrors::socketProtocolErrorStatuses[code] : closeMessage, code);
		_onSCError(err);
	}
}

void USCClientSocket::_onSCEvent(FString event, USCJsonObject* data, USCResponse* res)
{
	TFunction<void(USCJsonObject*, USCResponse*)> handler = _privateEventHandlerMap[event];
	if (handler)
	{
		handler(data, res);
	}
	else
	{
		Emitter.FindRef(event)(data, res);
	}
}

USCJsonObject* USCClientSocket::decode(FString message)
{
	return transport->decode(message);
}

FString USCClientSocket::encode(USCJsonObject* object)
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

	TFunction<void(USCJsonObject*, USCJsonObject*)> callback = eventObject->callback;
	if (callback)
	{
		eventObject->callback = nullptr;
		USCJsonObject* error = USCErrors::TimeoutError("Event response for '" + eventObject->event + "' timed out");
		callback(error, nullptr);
	}

	if (eventObject->cid != 0)
	{
		transport->cancelPendingResponse(eventObject->cid);
	}
}

void USCClientSocket::_emit(FString event, USCJsonObject* data, TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
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
	if (transport)
	{
		transport->send(data);
	}
}

void USCClientSocket::emitBlueprint(const FString& Event, USCJsonObject* Data, const FString& Callback, UObject* CallbackTarget)
{
	if (!Callback.IsEmpty())
	{
		emit(Event, Data, [&, Callback, CallbackTarget, this](USCJsonObject* error, USCJsonObject* data)
		{
			emitBlueprintCallback(CallbackTarget, Callback, error, data);
		});
	}
	else
	{
		emit(Event, Data);
	}
}

void USCClientSocket::emitBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error, USCJsonObject* data)
{
	if (!target->IsValidLowLevel())
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Emit target not found for callback function : %s"), *SCC_FUNC_LINE, *callback);
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Emit callback function '%s' not found"), *SCC_FUNC_LINE, *callback);
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

	if (FunctionParams == 0)
	{
		target->ProcessEvent(Function, nullptr);
	}
	else
	{

		struct FDynamicArgs
		{
			USCJsonObject* Arg01 = nullptr;
			USCJsonObject* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonObject*"))
		{
			Args.Arg01 = error;
		}
		else
		{
			UE_LOG(LogSCClient, Error, TEXT("%s : Emit callback function '%s' parameters incorrect"), *SCC_FUNC_LINE, *callback);
			return;
		}

		if (SecondParam.Equals("USCJsonObject*"))
		{
			Args.Arg02 = data;
		}

		target->ProcessEvent(Function, &Args);
	}
}

void USCClientSocket::emit(FString event, USCJsonObject* data, TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
{
	if (!_localEvents.Contains(event))
	{
		_emit(event, data, callback);
	}
	else if (event.Equals("error"))
	{
		Emitter.FindRef(event)(data, nullptr);
	}
	else
	{
		USCJsonObject* error = USCErrors::InvalidActionError("The '" + event + "' event is reserved and cannot be emitted on a client socket");
		_onSCError(error);
	}
}

void USCClientSocket::publishBlueprint(const FString& ChannelName, USCJsonObject* Data, const FString& Callback, UObject* CallbackTarget)
{
	if (!Callback.IsEmpty())
	{
		publish(ChannelName, Data, [&, Callback, CallbackTarget, this](USCJsonObject* error, USCJsonObject* data)
		{
			publishBlueprintCallback(CallbackTarget, Callback, error, data);
		});
	}
	else
	{
		publish(ChannelName, Data);
	}
}

void USCClientSocket::publishBlueprintCallback(UObject* target, const FString& callback, USCJsonObject* error, USCJsonObject* data)
{
	if (!target->IsValidLowLevel())
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Publish target not found for callback function : %s"), *SCC_FUNC_LINE, *callback);
		return;
	}

	UFunction* Function = target->FindFunction(FName(*callback));
	if (nullptr == Function)
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Publish callback function '%s' not found"), *SCC_FUNC_LINE, *callback);
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

	if (FunctionParams == 0)
	{
		target->ProcessEvent(Function, nullptr);
	}
	else
	{

		struct FDynamicArgs
		{
			USCJsonObject* Arg01 = nullptr;
			USCJsonObject* Arg02 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonObject*"))
		{
			Args.Arg01 = error;
		}
		else
		{
			UE_LOG(LogSCClient, Error, TEXT("%s : Publish callback function '%s' parameters incorrect"), *SCC_FUNC_LINE, *callback);
			return;
		}

		if (SecondParam.Equals("USCJsonObject*"))
		{
			Args.Arg02 = data;
		}

		target->ProcessEvent(Function, &Args);
	}
}

void USCClientSocket::publish(FString channelName, USCJsonObject* data, TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
{
	USCJsonObject* pubData = NewObject<USCJsonObject>();
	pubData->SetStringField("channel", _decorateChannelName(channelName));
	pubData->SetObjectField("data", data);
	emit("#publish", pubData, callback);
}

void USCClientSocket::_triggerChannelSubscribe(USCChannel* channel, USCJsonObject* subscriptionOptions)
{
	FString channelName = channel->_name;

	if (channel->_state != ESocketClusterChannelState::SUBSCRIBED)
	{
		ESocketClusterChannelState oldState = channel->_state;
		channel->_state = ESocketClusterChannelState::SUBSCRIBED;

		USCJsonObject* stateChangeData = NewObject<USCJsonObject>();
		stateChangeData->SetStringField("channel", channelName);
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", channel->_state));
		stateChangeData->SetObjectField("subscriptionOptions", subscriptionOptions);

		channel->OnChannelSubscribeStateChange.Broadcast(stateChangeData);
		channel->OnChannelSubscribe.Broadcast(channelName, subscriptionOptions);
		OnSubscribeStateChange.Broadcast(stateChangeData);
		OnSubscribe.Broadcast(channelName, subscriptionOptions);
	}
}

void USCClientSocket::_triggerChannelSubscribeFail(USCJsonObject * err, USCChannel * channel, USCJsonObject * subscriptionOptions)
{
	FString channelName = channel->_name;
	bool meetsAuthRequirements = !channel->_waitForAuth || authState == ESocketClusterAuthState::AUTHENTICATED;

	if (channel->_state != ESocketClusterChannelState::UNSUBSCRIBED && meetsAuthRequirements)
	{
		channel->_state = ESocketClusterChannelState::UNSUBSCRIBED;

		channel->OnChannelSubscribeFail.Broadcast(err, channelName, subscriptionOptions);;
		OnSubscribeFail.Broadcast(err, channelName, subscriptionOptions);
	}
}

void USCClientSocket::_cancelPendingSubscribeCallback(USCChannel* channel)
{
	if (channel->_pendingSubscriptionCid != 0)
	{
		transport->cancelPendingResponse(channel->_pendingSubscriptionCid);
		channel->_pendingSubscriptionCid = 0;
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

	bool meetsAuthRequirements = !channel->_waitForAuth || authState == ESocketClusterAuthState::AUTHENTICATED;

	if (state == ESocketClusterState::OPEN && !preparingPendingSubscriptions && channel->_pendingSubscriptionCid == 0 && meetsAuthRequirements)
	{

		USCJsonObject* opts = NewObject<USCJsonObject>();
		opts->SetBoolField("noTimeout", true);

		USCJsonObject* subscriptionOptions = NewObject<USCJsonObject>();
		subscriptionOptions->SetStringField("channel", _decorateChannelName(channel->_name));

		if (channel->_waitForAuth)
		{
			opts->SetBoolField("waitForAuth", true);
			subscriptionOptions->SetBoolField("waitForAuth", true);
		}

		if (channel->_data)
		{
			subscriptionOptions->SetObjectField("data", channel->_data);
		}

		if (channel->_batch)
		{
			opts->SetBoolField("batch", true);
			subscriptionOptions->SetBoolField("batch", true);
		}

		channel->_pendingSubscriptionCid = transport->emit("#subscribe", subscriptionOptions, opts, [&, channel](USCJsonObject* err, USCJsonObject* data)
		{
			channel->_pendingSubscriptionCid = 0;
			if (err)
			{
				_triggerChannelSubscribeFail(err, channel, subscriptionOptions);
			}
			else
			{
				_triggerChannelSubscribe(channel, subscriptionOptions);
			}
		});
		OnSubscribeRequest.Broadcast(channel->_name, subscriptionOptions);
	}
}

USCChannel* USCClientSocket::subscribe(const FString& channelName, const bool waitForAuth, USCJsonObject* data, const bool batch)
{

	USCJsonObject* opts = NewObject<USCJsonObject>();
	opts->SetBoolField("waitForAuth", waitForAuth);
	opts->SetObjectField("data", data);
	opts->SetBoolField("batch", batch);

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

	if (channel->_state == ESocketClusterChannelState::UNSUBSCRIBED)
	{
		channel->_state = ESocketClusterChannelState::PENDING;
		_trySubscribe(channel);
	}
	return channel;
}

void USCClientSocket::_triggerChannelUnsubscribe(USCChannel* channel, ESocketClusterChannelState newState)
{
	FString channelName = channel->_name;
	ESocketClusterChannelState oldState = channel->_state;

	channel->_state = newState;

	_cancelPendingSubscribeCallback(channel);

	if (oldState == ESocketClusterChannelState::SUBSCRIBED)
	{
		USCJsonObject* stateChangeData = NewObject<USCJsonObject>();
		stateChangeData->SetStringField("channel", channelName);
		stateChangeData->SetStringField("oldState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", oldState));
		stateChangeData->SetStringField("newState", USCJsonConvert::EnumToString<ESocketClusterChannelState>("ESocketClusterChannelState", channel->_state));

		channel->OnChannelSubscribeStateChange.Broadcast(stateChangeData);
		channel->OnChannelUnSubscribe.Broadcast(channelName);
		OnSubscribeStateChange.Broadcast(stateChangeData);
		OnUnSubscribe.Broadcast(channelName);
	}
}

void USCClientSocket::_tryUnsubscribe(USCChannel* channel)
{
	if (state == ESocketClusterState::OPEN)
	{
		USCJsonObject* opts = NewObject<USCJsonObject>();
		opts->SetBoolField("noTimeout", true);
		if (channel->_batch)
		{
			opts->SetBoolField("batch", true);
		}
		_cancelPendingSubscribeCallback(channel);

		FString decoratedChannelName = _decorateChannelName(channel->_name);
		USCJsonObject* data = NewObject<USCJsonObject>();
		data->DecodeJson(decoratedChannelName);
		transport->emit("#unsubscribe", data, opts);
	}
}

void USCClientSocket::unsubscribe(const FString& channelName)
{
	USCChannel* channel = channels.FindRef(channelName);

	if (channel)
	{
		if (channel->_state != ESocketClusterChannelState::UNSUBSCRIBED)
		{
			_triggerChannelUnsubscribe(channel);
			_tryUnsubscribe(channel);
		}
	}
}

USCChannel* USCClientSocket::channel(const FString& channelName, const bool waitForAuth, USCJsonObject* data, const bool batch)
{
	USCChannel* currentChannel = channels.FindRef(channelName);

	if (!currentChannel)
	{
		USCJsonObject* opts = NewObject<USCJsonObject>();
		opts->SetBoolField("waitForAuth", waitForAuth);
		opts->SetObjectField("data", data);
		opts->SetBoolField("batch", batch);
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
			includeChannel = (channel.Value->_state == ESocketClusterChannelState::SUBSCRIBED || channel.Value->_state == ESocketClusterChannelState::PENDING);
		}
		else
		{
			includeChannel = channel.Value->_state == ESocketClusterChannelState::SUBSCRIBED;
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
		return !!channel && (channel->_state == ESocketClusterChannelState::SUBSCRIBED || channel->_state == ESocketClusterChannelState::PENDING);
	}
	return !!channel && channel->_state == ESocketClusterChannelState::SUBSCRIBED;
}

void USCClientSocket::processPendingSubscriptions()
{

	preparingPendingSubscriptions = false;

	TArray<USCChannel*> pendingChannels = TArray<USCChannel*>();
	for (auto& channel : channels)
	{
		if(channel.Value->_state == ESocketClusterChannelState::PENDING)
		{
			pendingChannels.Add(channel.Value);
		}
	}

	for (auto& pendingChannel : pendingChannels)
	{
		_trySubscribe(pendingChannel);
	}
}

void USCClientSocket::watchBlueprint(const FString& channelName, const FString& Handler, UObject* HandlerTarget)
{
	if (!Handler.IsEmpty())
	{
		watch(channelName, [&, Handler, HandlerTarget, this](USCJsonObject* data)
		{
			watchBlueprintCallback(HandlerTarget, Handler, data);
		});
	}
	else
	{
		USCErrors::InvalidArgumentsError("No handler function was provided");
	}
}

void USCClientSocket::watchBlueprintCallback(UObject* target, const FString& handler, USCJsonObject* data)
{
	if (!target->IsValidLowLevel())
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Watch target not found for handler function : %s"), *SCC_FUNC_LINE, *handler);
		return;
	}

	UFunction* Function = target->FindFunction(FName(*handler));
	if (nullptr == Function)
	{
		UE_LOG(LogSCClient, Error, TEXT("%s : Watch handler function '%s' not found"), *SCC_FUNC_LINE, *handler);
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

	if (FunctionParams == 0)
	{
		target->ProcessEvent(Function, nullptr);
	}
	else
	{

		struct FDynamicArgs
		{
			USCJsonObject* Arg01 = nullptr;
		};

		FDynamicArgs Args = FDynamicArgs();

		const FString& FirstParam = Properties[0]->GetCPPType();
		const FString& SecondParam = Properties[1]->GetCPPType();

		if (FirstParam.Equals("USCJsonObject*"))
		{
			Args.Arg01 = data;
		}
		else
		{
			UE_LOG(LogSCClient, Error, TEXT("%s : Watch handler function '%s' parameters incorrect"), *SCC_FUNC_LINE, *handler);
			return;
		}

		target->ProcessEvent(Function, &Args);
	}
}

void USCClientSocket::watch(FString channelName, TFunction<void(USCJsonObject*)> handler)
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

TArray<TFunction<void(USCJsonObject*)>> USCClientSocket::watchers(FString channelName)
{
	TArray<TFunction<void(USCJsonObject*)>> watchers;
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

FString USCClientSocket::queryParse(TArray<USCJsonObject*> query)
{
	FString querystring;
	if (query.Num() > 0)
	{
		for (auto& it : query)
		{
			if (querystring.IsEmpty())
			{
				querystring.Append(TEXT("?")).Append(it->GetStringField("key")).Append(TEXT("=")).Append(it->GetStringField("value"));
			}
			else
			{
				querystring.Append(TEXT("&")).Append(it->GetStringField("key")).Append(TEXT("=")).Append(it->GetStringField("value"));
			}
		}
	}
	return querystring;
}