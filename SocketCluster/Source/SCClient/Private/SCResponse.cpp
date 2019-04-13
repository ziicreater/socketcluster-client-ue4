// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCResponse.h"
#include "SCErrors.h"
#include "SCClientModule.h"
#include "SCTransport.h"

void USCResponse::create(USCTransport* transport, int32 cid)
{
	this->socket = transport;
	this->id = cid;
	this->sent = false;
}

void USCResponse::_respond(USCJsonObject* responseData)
{
	if (this->sent)
	{
		USCErrors::InvalidActionError("Response " + FString::FromInt(this->id) + " has already been sent");
	}
	else
	{
		this->sent = true;
		this->socket->send(this->socket->encode(responseData));
	}
}

void USCResponse::end(USCJsonObject* data)
{
	if (this->id != 0)
	{
		USCJsonObject* responseData = NewObject<USCJsonObject>();
		responseData->SetIntegerField("rid", this->id);
		if (data)
		{
			responseData->SetObjectField("data", data);
		}
		this->_respond(responseData);
	}
}

void USCResponse::error(USCJsonObject* error, USCJsonObject* data)
{
	if (this->id != 0)
	{
		USCJsonObject* err = USCErrors::Error(error);

		USCJsonObject* responseData = NewObject<USCJsonObject>();
		responseData->SetIntegerField("rid", this->id);
		responseData->SetObjectField("error", err);

		if (data)
		{
			responseData->SetObjectField("data", data);
		}
		this->_respond(responseData);
	}
}

void USCResponse::callback(USCJsonObject* error, USCJsonObject* data)
{
	UE_LOG(LogSCClient, Error, TEXT("%s :USCResponse::Callback Data : %s"), *SCC_FUNC_LINE, *data->EncodeJson());
	if (error)
	{
		this->error(error, data);
	}
	else
	{
		this->end(data);
	}
}

void USCResponse::res(USCJsonObject* Error, USCJsonObject* Data)
{
	this->callback(Error, Data);
}
