// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCTransport.h"
#include "SCAuthEngine.h"
#include "SCErrors.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "SCClientModule.h"

void USCTransport::BeginDestroy()
{
	Super::BeginDestroy();
}

UWorld* USCTransport::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void USCTransport::create(USCAuthEngine* authEngine, USCCodecEngine* codecEngine, USCJsonObject* opts)
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

	socket->onclose = [&](const USCJsonObject* Event)
	{
		int32 code;
		if (!Event->HasField("code"))
		{
			code = 1005;
		}
		else
		{
			code = Event->GetIntegerField("code");
		}
		_onClose(code, Event->GetStringField("reason"));
	};

	socket->onmessage = [&](const USCJsonObject* Message)
	{
		_onMessage(Message->GetStringField("data"));
	};

	socket->onerror = [&](const USCJsonObject* Error)
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

	_handshake([&](USCJsonObject* err, USCJsonObject* status)
	{
		if (err)
		{
			int32 statusCode;
			if (status && status->HasField("code"))
			{
				statusCode = status->GetIntegerField("code");
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
			onopen(status);
			_resetPingTimeout();
		}
	});
}

void USCTransport::_handshake(TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
{

	auth->loadToken(authTokenName, [&](USCJsonObject* err, FString& token)
	{
		if (err)
		{
			callback(err, nullptr);
		}
		else
		{
			USCJsonObject* options = NewObject<USCJsonObject>();
			options->SetBoolField("force", true);

			USCJsonObject* data = NewObject<USCJsonObject>();
			if (!token.IsEmpty())
			{
				data->SetStringField("authToken", token);
			}
			emit("#handshake", data, options, [&, token, callback, this](USCJsonObject* err, USCJsonObject* status)
			{
				if (status)
				{
					if (!token.IsEmpty())
					{
						status->SetStringField("authToken", token);
					}

					if (status->HasField("authError"))
					{
						status->SetObjectField("authError", USCErrors::Error(status->GetObjectField("authError")));
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
		USCJsonObject* badConnectionError = USCErrors::BadConnectionError(errorMessage, failureType);

		TFunction<void(USCJsonObject* error, USCJsonObject* data)> callback = eventObject->callback;
		callback(badConnectionError, eventObject->data);
	}
}

void USCTransport::_onClose(int32 code, FString data)
{
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

void USCTransport::_handleEventObject(USCJsonObject* obj, FString message)
{
	if (obj && obj->HasField("event"))
	{
		USCResponse* response = NewObject<USCResponse>();
		response->create(this, obj->GetIntegerField("cid"));
		onevent(obj->GetStringField("event"), obj->GetObjectField("data"), response);
	}
	else if (obj && obj->HasField("rid"))
	{
		USCEventObject* eventObject = _callbackMap.FindRef(obj->GetIntegerField("rid"));
		if (eventObject)
		{
			clearTimeout(eventObject->timeoutHandle);
			_callbackMap.Remove(obj->GetIntegerField("rid"));
			if (eventObject->callback)
			{
				USCJsonObject* rehydratedError = USCErrors::Error(obj->GetObjectField("error"));
				eventObject->callback(rehydratedError, obj->GetObjectField("data"));
			}
		}
	}
	else
	{
		onraw(message);
	}
}

void USCTransport::_onMessage(FString message)
{
	onmessage(message);
	
	if (options->GetIntegerField("protocolVersion") == 0 && message.Equals("#1"))
	{
		_resetPingTimeout();
		if (socket->readyState == ESocketClusterState::OPEN)
		{
			send("#2");
		}
	}
	else if(options->GetIntegerField("protocolVersion") == 1 && message.Equals(""))
	{
		_resetPingTimeout();
		if (socket->readyState == ESocketClusterState::OPEN)
		{
			send("");
		}
	}
	else
	{
		FString MessageArray = FString("{\"root\":");
		MessageArray.Append(message).Append("}");
		USCJsonObject* DecodedArray = decode(MessageArray);
		TArray<USCJsonObject*> ArrayObj = DecodedArray->GetObjectArrayField("root");

		if (ArrayObj.Num() > 0)
		{
			for (int32 i = 0; i != ArrayObj.Num(); ++i)
			{
				_handleEventObject(ArrayObj[i], message);
			}
		}
		else
		{
			USCJsonObject* obj = decode(message);
			this->_handleEventObject(obj, message);
		}
	}
}

void USCTransport::_onError(USCJsonObject* err)
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

void USCTransport::close(int32 code, USCJsonObject* data)
{
	if (state == ESocketClusterState::OPEN)
	{

		USCJsonObject* packet = NewObject<USCJsonObject>();
		packet->SetIntegerField("code", code);
		if (data)
		{
			packet->SetObjectField("data", data);
		}

		emit("#disconnect", packet);

		_onClose(code, data ? data->EncodeJson() : "");
		socket->close(code);

		onopen = nullptr;
		onerror = nullptr;
		onclose = nullptr;
		onopenAbort = nullptr;
		onevent = nullptr;
		onraw = nullptr;
		onmessage = nullptr;
		clearTimeout(_pingTimeoutTickerHandle);
	}
	else if (state == ESocketClusterState::CONNECTING)
	{
		_onClose(code, data ? data->EncodeJson() : "");
		socket->close(code);
	}
}

int32 USCTransport::emitObject(USCEventObject* eventObject, USCJsonObject* opts)
{

	USCJsonObject* simpleEventObject = NewObject<USCJsonObject>();
	simpleEventObject->SetStringField("event", eventObject->event);
	simpleEventObject->SetObjectField("data", eventObject->data);

	if (eventObject->callback)
	{
		eventObject->cid = callIdGenerator();
		simpleEventObject->SetIntegerField("cid", eventObject->cid);
		_callbackMap.Add(eventObject->cid, eventObject);
	}

	sendObject(simpleEventObject, opts);

	return eventObject->cid || 0;
}

void USCTransport::_handleEventAckTimeout(USCEventObject* eventObject)
{
	if (eventObject->cid != 0)
	{
		_callbackMap.Remove(eventObject->cid);
	}

	clearTimeout(eventObject->timeoutHandle);

	TFunction<void(USCJsonObject* error, USCJsonObject* data)> callback = eventObject->callback;
	if (callback)
	{
		USCJsonObject* error = USCErrors::TimeoutError("Event response for '" + eventObject->event + "' timed out");
		callback(error, eventObject->data);
	}
}

int32 USCTransport::emit(FString event, USCJsonObject* data, USCJsonObject* opts, TFunction<void(USCJsonObject*, USCJsonObject*)> callback)
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

USCJsonObject* USCTransport::decode(FString message)
{
	return codec->decode(message);
}

FString USCTransport::encode(USCJsonObject* object)
{
	return codec->encode(object);
}

void USCTransport::send(FString data)
{
	if (socket->readyState != ESocketClusterState::OPEN)
	{
		_onClose(1005);
	}
	else
	{
		socket->send(data);
	}
}

FString USCTransport::serializeObject(USCJsonObject* object)
{
	FString str = encode(object);
	return str;
}

FString USCTransport::serializeObject(TSet<USCJsonObject*> object)
{
	FString str;
	for (auto& Entry : object)
	{
		if (str.IsEmpty())
		{
			str.Append("[").Append(encode(Entry));
		}
		else
		{
			str.Append(",").Append(encode(Entry));
		}
	}
	if (!str.IsEmpty()) { str.Append("]"); }
	return str;
}

void USCTransport::sendObjectBatch(USCJsonObject* object)
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
			FString str = serializeObject(_batchSendList);
			if (!str.IsEmpty())
			{
				send(str);
			}
			_batchSendList.Empty();
		}
	});
	GetWorld()->GetTimerManager().SetTimer(_batchTimeoutHandle, _batchTimeout, options->GetNumberField("pubSubBatchDuration"), false);
}

void USCTransport::sendObjectSingle(USCJsonObject* object)
{
	FString str = serializeObject(object);
	if (!str.IsEmpty())
	{
		send(str);
	}
}

void USCTransport::sendObject(USCJsonObject* object, USCJsonObject* opts)
{
	if (opts && opts->HasField("batch"))
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

void USCTransport::clearTimeout(FTimerHandle timer)
{
	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(this, timer))
	{
		GetWorld()->GetTimerManager().ClearTimer(timer);
	}
}