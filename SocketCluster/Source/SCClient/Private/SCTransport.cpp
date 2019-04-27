// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCTransport.h"
#include "SCAuthEngine.h"
#include "SCErrors.h"
#include "SCSocket.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "SCClientModule.h"

void USCTransport::BeginDestroy()
{
	off();
	Super::BeginDestroy();
}

UWorld* USCTransport::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void USCTransport::create(USCAuthEngine* authEngine, USCCodecEngine* codecEngine, TSharedPtr<FJsonObject> opts)
{
	state = ESocketClusterState::CLOSED;
	auth = authEngine;
	codec = codecEngine;
	options = opts;
	connectTimeout = opts->GetNumberField("connectTimeout");
	pingTimeout = opts->GetNumberField("ackTimeout");
	pingTimeoutDisabled = opts->GetBoolField("pingTimeoutDisabled");
	_cid = opts->GetIntegerField("callIdGenerator");
	authTokenName = opts->GetStringField("authTokenName");

	clearTimeout(_pingTimeoutTickerHandle);
	_callbackMap.Empty();
	_batchSendList.Empty();


	state = ESocketClusterState::CONNECTING;
	FString url = uri();

	socket = NewObject<USCSocket>(this);
	socket->createWebSocket(url, options);
	
	socket->onopen = [&]()
	{
		_onOpen();
	};

	socket->onclose = [&](const TSharedPtr<FJsonObject> Event)
	{
		int32 code;
		if (!Event->HasField("code"))
		{
			code = 1005;
		}
		else
		{
			code = Event->GetNumberField("code");
		}
		_onClose(code, Event->GetStringField("reason"));
	};

	socket->onmessage = [&](const TSharedPtr<FJsonObject> Message)
	{
		_onMessage(Message->GetStringField("data"));
	};

	socket->onerror = [&](const TSharedPtr<FJsonValue> Error)
	{
		if (state == ESocketClusterState::CONNECTING)
		{
			_onClose(1006);
		}
	};

	_connectTimeoutRef.BindLambda([&]()
	{
		_onClose(4007);
		socket->close(4007);
	});
	GetWorld()->GetTimerManager().SetTimer(_connectTimeoutHandle,_connectTimeoutRef, connectTimeout, false);
}

FString USCTransport::uri()
{
	FString query = options->GetStringField("query");
	FString schema = options->GetBoolField("secure") ? "wss" : "ws";


	if (options->HasField("timestampRequests") && options->GetBoolField("timestampRequests"))
	{
		if (query.IsEmpty())
		{
			query.Append(TEXT("?")).Append(options->GetStringField("timestampParam")).Append(TEXT("=")).Append(FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp() / 1000));
		}
		else
		{
			query.Append(TEXT("&")).Append(options->GetStringField("timestampParam")).Append(TEXT("=")).Append(FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp() / 1000));
		}
	}
	
	FString port;
	if (options->HasField("port") && ((schema.Equals("wss") && options->GetIntegerField("port") != 443) || (schema.Equals("ws") && options->GetIntegerField("port") != 80)))
	{
		port = ":" + FString::FromInt(options->GetIntegerField("port"));
	}
	FString host = options->GetStringField("hostname") + port;

	return schema + "://" + host + options->GetStringField("path") + query;
}

void USCTransport::_onOpen()
{
	clearTimeout(_connectTimeoutHandle);
	_resetPingTimeout();

	_handshake([&](TSharedPtr<FJsonValue> err, TSharedPtr<FJsonValue> status)
	{
		if (err.IsValid())
		{
			int32 statusCode;
			if (status.IsValid() && status->AsObject()->HasField("code"))
			{
				statusCode = status->AsObject()->GetNumberField("code");
			}
			else
			{
				statusCode = 4003;
			}
			_onError(err);
			_onClose(statusCode);
			socket->close(statusCode);
		}
		else
		{
			state = ESocketClusterState::OPEN;
			onopen(status->AsObject());
			_resetPingTimeout();
		}
	});
}

void USCTransport::_handshake(TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{
	auth->loadToken(authTokenName, [&, callback](TSharedPtr<FJsonValue> err, FString token)
	{
		if (err.IsValid())
		{
			callback(err, nullptr);
		}
		else
		{
			TSharedPtr<FJsonObject> options = MakeShareable(new FJsonObject);
			options->SetBoolField("force", true);

			TSharedPtr<FJsonObject> data = MakeShareable(new FJsonObject);
			if (!token.IsEmpty())
			{
				data->SetStringField("authToken", token);
			}
			emit("#handshake", USCJsonConvert::ToJsonValue(data), options, [&, token, callback, this](TSharedPtr<FJsonValue> err, TSharedPtr<FJsonValue> status)
			{
				if (status.IsValid())
				{
					TSharedPtr<FJsonObject> data = status->AsObject();
					if (!token.IsEmpty())
					{
						data->SetStringField("authToken", token);
					}

					if (data->HasField("authError"))
					{
						data->SetField("authError", USCErrors::Error(USCJsonConvert::ToJsonValue(data->GetObjectField("authError"))));
					}
				}
				callback(err, status);
			});
		}
	});
}

void USCTransport::_abortAllPendingEventsDueToBadConnection(FString failureType)
{
	TMap<int32, USCEventObject*> _callbackMaplocal = _callbackMap;
	for (auto& i : _callbackMaplocal)
	{
		USCEventObject* eventObject = _callbackMap.FindAndRemoveChecked(i.Key);

		clearTimeout(eventObject->timeoutHandle);

		FString errorMessage = "Event '" + eventObject->event + "' was aborted due to a bad connection";
		TSharedPtr<FJsonValue> badConnectionError = USCErrors::BadConnectionError(errorMessage, failureType);

		TFunction<void(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)> callback = eventObject->callback;
		callback(badConnectionError, eventObject->data);
	}
}

void USCTransport::_onClose(int32 code, FString data)
{
	socket->onopen = nullptr;
	socket->onclose = nullptr;
	socket->onmessage = nullptr;
	socket->onerror = nullptr;

	clearTimeout(_connectTimeoutHandle);
	clearTimeout(_pingTimeoutTickerHandle);
	clearTimeout(_batchTimeoutHandle);

	if (state == ESocketClusterState::OPEN)
	{
		state = ESocketClusterState::CLOSED;
		onclose(code, data);
		_abortAllPendingEventsDueToBadConnection("disconnect");
	}
	else if (state == ESocketClusterState::CONNECTING)
	{
		state = ESocketClusterState::CLOSED;
		onopenAbort(code, data);
		_abortAllPendingEventsDueToBadConnection("disconnect");
	}
}

void USCTransport::_handleEventObject(TSharedPtr<FJsonObject> obj, FString message)
{
	if (obj.IsValid() && obj->HasField("event"))
	{
		USCResponse* response = nullptr;
		if (obj->HasField("cid"))
		{
			response = NewObject<USCResponse>();
			response->create(this, obj->GetNumberField("cid"));
		}
		onevent(obj->GetStringField("event"), USCJsonConvert::ToJsonValue(obj->GetObjectField("data")), response);
	}
	else if (obj.IsValid() && obj->HasField("rid"))
	{
		USCEventObject* eventObject = _callbackMap.FindRef(obj->GetNumberField("rid"));
		if (eventObject)
		{
			clearTimeout(eventObject->timeoutHandle);
			_callbackMap.Remove(obj->GetNumberField("rid"));
			if (eventObject->callback)
			{
				TSharedPtr<FJsonValue> rehydratedError;
				if (obj->HasField("error") && obj->GetObjectField("error").IsValid())
				{
					rehydratedError = USCErrors::Error(USCJsonConvert::ToJsonValue(obj->GetObjectField("error")));
				}

				TSharedPtr<FJsonValue> data = MakeShareable(new FJsonValueNull);
				if (obj->HasField("data") && obj->GetObjectField("data"))
				{
					data = USCJsonConvert::ToJsonValue(obj->GetObjectField("data"));
				}

				eventObject->callback(rehydratedError, data);
			}
		}
	}
	else
	{
		onevent("raw", USCJsonConvert::ToJsonValue(message), nullptr);
	}
}

void USCTransport::_onMessage(FString message)
{
	onevent("message", USCJsonConvert::ToJsonValue(message), nullptr);

	TSharedPtr<FJsonValue> obj = decode(message);

	if (options->GetNumberField("protocolVersion") == 0 && obj->Type == EJson::String && obj->AsString().Equals("#1"))
	{
		_resetPingTimeout();
		if (socket->readyState == ESocketState::OPEN)
		{
			sendObject(MakeShareable(new FJsonValueString("#2")));
		}
	}
	else if(options->GetNumberField("protocolVersion") == 1 && obj->Type == EJson::Null && obj->IsNull())
	{
		_resetPingTimeout();
		if (socket->readyState == ESocketState::OPEN)
		{
			sendObject(MakeShareable(new FJsonValueString("")));
		}
	}
	else
	{
		if (obj->Type == EJson::Array)
		{
			TArray<TSharedPtr<FJsonValue>> array = obj->AsArray();
			for (int32 i = 0; i != array.Num(); ++i)
			{
				TSharedPtr<FJsonObject> objItem = array[i]->AsObject();
				_handleEventObject(objItem, message);
			}
		}
		else
		{
			TSharedPtr<FJsonObject> Item = obj->AsObject();
			_handleEventObject(Item, message);
		}	
	}
}

void USCTransport::_onError(TSharedPtr<FJsonValue> err)
{
	onerror(err);
}

void USCTransport::_resetPingTimeout()
{
	if (pingTimeoutDisabled)
	{
		return;
	}

	clearTimeout(_pingTimeoutTickerHandle);

	_pingTimeoutTicker.BindLambda([this]() {
		_onClose(4000);
		socket->close(4000);
	});
	GetWorld()->GetTimerManager().SetTimer(_pingTimeoutTickerHandle, _pingTimeoutTicker, pingTimeout, false);
}

void USCTransport::close(int32 code, TSharedPtr<FJsonValue> data)
{
	if (state == ESocketClusterState::OPEN)
	{
		TSharedPtr<FJsonObject> packet = MakeShareable(new FJsonObject);
		packet->SetNumberField("code", code);
		if (data.IsValid())
		{
			packet->SetField("data", data);
		}

		emit("#disconnect", USCJsonConvert::ToJsonValue(packet));

		_onClose(code, data ? USCJsonConvert::ToJsonString(data) : "");
		socket->close(code);
	}
	else if (state == ESocketClusterState::CONNECTING)
	{
		_onClose(code, data ? USCJsonConvert::ToJsonString(data) : "");
		socket->close(code);
	}
}

int32 USCTransport::emitObject(USCEventObject* eventObject, TSharedPtr<FJsonObject> opts)
{
	TSharedPtr<FJsonObject> simpleEventObject = MakeShareable(new FJsonObject);
	simpleEventObject->SetStringField("event", eventObject->event);
	simpleEventObject->SetField("data", eventObject->data);

	if (eventObject->callback)
	{
		eventObject->cid = callIdGenerator();
		simpleEventObject->SetNumberField("cid", eventObject->cid);
		_callbackMap.Add(eventObject->cid, eventObject);
	}

	sendObject(USCJsonConvert::ToJsonValue(simpleEventObject), opts);

	return eventObject->cid || 0;
}

void USCTransport::_handleEventAckTimeout(USCEventObject* eventObject)
{
	if (eventObject->cid != 0)
	{
		_callbackMap.Remove(eventObject->cid);
	}

	clearTimeout(eventObject->timeoutHandle);

	TFunction<void(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)> callback = eventObject->callback;
	if (callback)
	{
		TSharedPtr<FJsonValue> error = USCErrors::TimeoutError("Event response for '" + eventObject->event + "' timed out");
		callback(error, eventObject->data);
	}
}

int32 USCTransport::emit(FString event, TSharedPtr<FJsonValue> data, TSharedPtr<FJsonObject> opts, TFunction<void(TSharedPtr<FJsonValue>, TSharedPtr<FJsonValue>)> callback)
{

	USCEventObject* eventObject = NewObject<USCEventObject>();
	eventObject->event = event;
	eventObject->data = data;
	eventObject->callback = callback;

	if (callback && !opts->HasField("noTimeout"))
	{
		eventObject->timeout = FTimerDelegate::CreateUObject(this, &USCTransport::_handleEventAckTimeout, eventObject);
		GetWorld()->GetTimerManager().SetTimer(eventObject->timeoutHandle, eventObject->timeout, options->GetNumberField("ackTimeout"), false);
	}

	int32 cid = 0;
	if (state == ESocketClusterState::OPEN || opts->GetBoolField("force"))
	{
		cid = emitObject(eventObject, opts);
	}
	return cid;
}

void USCTransport::cancelPendingResponse(int32 cid)
{
	_callbackMap.Remove(cid);
}

TSharedPtr<FJsonValue> USCTransport::decode(FString message)
{
	return codec->decode(message);
}

FString USCTransport::encode(TSharedPtr<FJsonValue> object)
{
	return codec->encode(object);
}

void USCTransport::send(FString data)
{
	if (socket->readyState != ESocketState::OPEN)
	{
		_onClose(1005);
	}
	else
	{
		socket->send(data);
	}
}

FString USCTransport::serializeObject(TSharedPtr<FJsonValue> object)
{
	FString str = encode(object);
	return str;
}

void USCTransport::sendObjectBatch(TSharedPtr<FJsonValue> object)
{
	_batchSendList.Add(object);
	if (GetWorld()->GetTimerManager().IsTimerActive(_batchTimeoutHandle))
	{
		return;
	}

	_batchTimeout.BindLambda([&]()
	{
		clearTimeout(_batchTimeoutHandle);
		if (_batchSendList.Num() > 0)
		{
			FString str = serializeObject(USCJsonConvert::ToJsonValue(_batchSendList));
			if (!str.IsEmpty())
			{
				send(str);
			}
			_batchSendList.Empty();
		}
	});
	GetWorld()->GetTimerManager().SetTimer(_batchTimeoutHandle, _batchTimeout, options->GetNumberField("pubSubBatchDuration") || 0, false);
}

void USCTransport::sendObjectSingle(TSharedPtr<FJsonValue> object)
{
	FString str = serializeObject(object);
	send(str);
}

void USCTransport::sendObject(TSharedPtr<FJsonValue> object, TSharedPtr<FJsonObject> opts)
{
	if (opts.IsValid() && opts->HasField("batch"))
	{
		sendObjectBatch(object);
	}
	else
	{
		sendObjectSingle(object);
	}
}

int32 USCTransport::callIdGenerator()
{
	return _cid++;
}

void USCTransport::off()
{
	clearTimeout(_batchTimeoutHandle);
	clearTimeout(_connectTimeoutHandle);
	clearTimeout(_pingTimeoutTickerHandle);
}

void USCTransport::clearTimeout(FTimerHandle timer)
{
	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(this, timer))
	{
		GetWorld()->GetTimerManager().ClearTimer(timer);
	}
}