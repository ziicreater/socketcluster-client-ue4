// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCResponse.h"
#include "SCErrors.h"
#include "SCClientModule.h"
#include "SCTransport.h"

void USCResponse::create(USCTransport* transport, int32 cid)
{
	socket = transport;
	id = cid;
	sent = false;
}

void USCResponse::_respond(TSharedPtr<FJsonValue> responseData)
{
	if (sent)
	{
		USCErrors::InvalidActionError("Response " + FString::FromInt(id) + " has already been sent");
	}
	else
	{
		sent = true;
		socket->send(socket->encode(responseData));
	}
}

void USCResponse::end(TSharedPtr<FJsonValue> data)
{
	if (id != 0)
	{
		TSharedPtr<FJsonObject> responseData = MakeShareable(new FJsonObject);
		responseData->SetNumberField("rid", id);
		if (data)
		{
			responseData->SetField("data", data);
		}
		_respond(USCJsonConvert::ToJsonValue(responseData));
	}
}

void USCResponse::error(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
{
	if (id != 0)
	{
		TSharedPtr<FJsonValue> err = USCErrors::Error(error);

		TSharedPtr<FJsonObject> responseData = MakeShareable(new FJsonObject);
		responseData->SetNumberField("rid", id);
		responseData->SetField("error", err);

		if (data)
		{
			responseData->SetField("data", data);
		}
		_respond(USCJsonConvert::ToJsonValue(responseData));
	}
}

void USCResponse::callback(TSharedPtr<FJsonValue> err, TSharedPtr<FJsonValue> data)
{
	if (err)
	{
		error(err, data);
	}
	else
	{
		end(data);
	}
}

void USCResponse::res(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data)
{
	callback(error, data);
}

void USCResponse::resBlueprint(USCJsonValue* error, USCJsonValue* data)
{
	TSharedPtr<FJsonValue> errorValue = nullptr;
	if (error != nullptr)
	{
		errorValue = error->GetRootValue();
	}

	TSharedPtr<FJsonValue> dataValue = nullptr;
	if (data != nullptr)
	{
		dataValue = data->GetRootValue();
	}

	res(errorValue, dataValue);
}
