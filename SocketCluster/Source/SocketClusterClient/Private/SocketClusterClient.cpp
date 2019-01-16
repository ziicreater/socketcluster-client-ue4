// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#include "SocketClusterClient.h"
#include "SocketClusterContext.h"
#include "SocketClusterClientModule.h"

// Namespace UI Conflict.
// Remove UI Namepspace
#if PLATFORM_LINUX
#pragma push_macro("UI")
#undef UI
#elif PLATFORM_WINDOWS || PLATFORM_MAC
#define UI UI_ST
#endif 

THIRD_PARTY_INCLUDES_START
#include <iostream>
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END

// Namespace UI Conflict.
// Restore UI Namepspace
#if PLATFORM_LINUX
#pragma pop_macro("UI")
#elif PLATFORM_WINDOWS || PLATFORM_MAC
#undef UI
#endif 

TSharedPtr<USocketClusterContext> _SocketClusterContext;

/* Set default settings for SocketCluster Client */
USocketClusterClient::USocketClusterClient()
{

	/* Set this socket cluster client as actice */
	this->active = true;

	/* Set the current id of the socket to null when first creating a instance */
	this->id = NULL;

	/* Set the first call id to 1 */
	this->cid = 1;

	/* Set the current state of the socket to closed when first creating a instance */
	this->state = ESocketClusterState::CLOSED;

	/* Set the current authState of the socket to unauthenticated when first creating a instance */
	this->authState = ESocketClusterAuthState::UNAUTHENTICATED;

	/* Set the current signedAuthToken of the socket to null when first creating a instance */
	this->signedAuthToken = NULL;
	
	/* Set the current authToken of the socket to null when first creating a instance */
	this->authToken = NULL;

	/* Set the current pendingReconnect of the socket to false when first creating a instance */
	this->pendingReconnect = false;

	/* Set the current pendingReconnectTimeout of the socket to false when first creating a instance */
	this->pendingReconnectTimeout = false;

	/* Set the current preparingPendingSubscriptions of the socket to false when first creating a instance */
	this->preparingPendingSubscriptions = false;

	/* Set the current pingTimeoutDisabled of the socket to false when first creating a instance */
	this->pingTimeoutDisabled = false;

	/* Set the current connectAttempts of the socket to 0 when first creating a instance */
	this->connectAttempts = 0;

}

void USocketClusterClient::OnPublish(const FOnPublish& event)
{
	OnPublishCallback = event;
}

/*
void USocketClusterClient::OnAuthToken(const FOnAuthToken& event)
{
	OnAuthTokenCallback = event;
}
*/

/* Override the BeginDestroy function from the object */
void USocketClusterClient::BeginDestroy()
{
	if (context != nullptr)
	{
		lws_context_destroy(context);
		context = nullptr;
		_SocketClusterContext.Reset();
	}
	Super::BeginDestroy();
}

/* Call Id Generator */
int32 USocketClusterClient::callIdGenerator()
{
	return this->cid++;
}

/* Create an instance of SocketClusterClient */
USocketClusterClient* USocketClusterClient::Create
(
	const TArray<FSocketClusterKeyValue>& Query,
	UObject* Codec,
	const FString& uri,
	const bool AutoConnect,
	const bool AutoReconnect,
	const float ReconnectInitialDelay,
	const float ReconnectRandomness,
	const float ReconnectMultiplier,
	const float ReconnectMaxDelay,
	const bool AutoSubscribeOnConnect,
	const float ConnectTimeout,
	const float AckTimeout,
	const bool TimestampRequests,
	const FString& TimestampParam
)
{
	if (_SocketClusterContext.Get() == nullptr)
	{
		_SocketClusterContext = MakeShareable(NewObject<USocketClusterContext>());
		_SocketClusterContext->CreateContext();
		_SocketClusterContext->AddToRoot();
	}

	return _SocketClusterContext->Create(Query, Codec, uri, AutoConnect, AutoReconnect, ReconnectInitialDelay, ReconnectRandomness, ReconnectMultiplier, ReconnectMaxDelay, AutoSubscribeOnConnect, ConnectTimeout, AckTimeout, TimestampRequests, TimestampParam);
}

/* Connect to the SocketCluster Server */
void USocketClusterClient::Connect()
{

	if (!this->active)
	{
		UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Cannot connect a destroyed client"), *SCC_FUNC_LINE);
		return;
	}

	if (this->state == ESocketClusterState::CLOSED)
	{
		this->pendingReconnect = false;
		this->pendingReconnectTimeout = NULL;

		this->state = ESocketClusterState::CONNECTING;

		/* Check if we have a context */
		if (context == nullptr)
		{
			return;
		}
		
		int iUseSSL = 0;
		int iPos = uri.Find(TEXT(":"));
		if (iPos == INDEX_NONE)
		{
			UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Wrong Uri Format"), *SCC_FUNC_LINE);
			return;
		}

		FString strProtocol = uri.Left(iPos);
		if (strProtocol.ToUpper() != TEXT("WS") && strProtocol.ToUpper() != TEXT("WSS") && strProtocol.ToUpper() != TEXT("HTTP") && strProtocol.ToUpper() != TEXT("HTTPS"))
		{
			UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Protocol not supported"), *SCC_FUNC_LINE);
			return;
		}

		if (strProtocol.ToUpper() == TEXT("WSS") || strProtocol.ToUpper() == TEXT("HTTPS"))
		{
			iUseSSL = 2;
		}

		FString strHost;
		FString strPath = TEXT("/");
		FString strNextParse = uri.Mid(iPos + 3);
		iPos = strNextParse.Find("/");
		if (iPos != INDEX_NONE)
		{
			strHost = strNextParse.Left(iPos);
			strPath = strNextParse.Mid(iPos);
		}
		else
		{
			strHost = strNextParse;
		}

		FString strAddress = strHost;
		int iPort = 80;
		iPos = strAddress.Find(":");
		if (iPos != INDEX_NONE)
		{
			strAddress = strHost.Left(iPos);
			iPort = FCString::Atoi(*strHost.Mid(iPos + 1));
		}
		else
		{
			if (iUseSSL)
			{
				iPort = 443;
			}
		}

		if (bTimestampRequests)
		{
			FSocketClusterKeyValue timestampRequests;
			timestampRequests.key = timestampParam;
			timestampRequests.value = FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp() / 1000);
			query.Add(timestampRequests);
		}

		FString queryString;
		if (query.Num() > 0)
		{
			for (auto& it : query)
			{
				//std::string strKey = TCHAR_TO_UTF8(*(it.key));
				//std::string strValue = TCHAR_TO_UTF8(*(it.value));
				
				if (queryString.IsEmpty())
				{
					queryString.Append(TEXT("?")).Append(it.key).Append(TEXT("=")).Append(it.value);
					//queryString = "?" + it.key + "=" + it.value;
				}
				else
				{
					queryString.Append(TEXT("&")).Append(it.key).Append(TEXT("=")).Append(it.value);
					//queryString = queryString + "&" + it.key + "=" + it.value;
				}

			}
			strHost.Append(queryString);
			//strAddress = strAddress + queryString;
			UE_LOG(LogSocketClusterClient, Error, TEXT("%s : queryString = %s"), *SCC_FUNC_LINE, *strHost);
		}
		

		struct lws_client_connect_info info;
		memset(&info, 0, sizeof(info));

		std::string stdAddress = TCHAR_TO_UTF8(*strAddress);
		std::string stdPath = TCHAR_TO_UTF8(*strPath);
		std::string stdHost = TCHAR_TO_UTF8(*strHost);

		/* Set connection info */
		info.context = context;
		info.address = stdAddress.c_str();
		info.port = iPort;
		info.ssl_connection = iUseSSL;
		info.path = stdPath.c_str();
		info.host = stdHost.c_str();
		info.origin = stdHost.c_str();
		info.ietf_version_or_minus_one = -1;
		info.userdata = this;

		lws = lws_client_connect_via_info(&info);

		if (lws == nullptr)
		{
			UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Error trying to create client connecton."), *SCC_FUNC_LINE);
			return;
		}

	}
}

/* Subscribe to a channel */
void USocketClusterClient::Subscribe(const FString& channel)
{
	USocketClusterJsonObject* SocketClusterJsonObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonObject->SetStringField(FString(TEXT("event")), FString(TEXT("#subscribe")));

	USocketClusterJsonObject* SocketClusterJsonDataObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonDataObject->SetStringField("channel", channel);

	SocketClusterJsonObject->SetObjectField("data", SocketClusterJsonDataObject);

	Send(SocketClusterJsonObject);
}

void USocketClusterClient::UnSubscribe(const FString& channel)
{
	USocketClusterJsonObject* SocketClusterJsonObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonObject->SetStringField(FString(TEXT("event")), FString(TEXT("#unsubscribe")));
	SocketClusterJsonObject->SetStringField("data", channel);

	Send(SocketClusterJsonObject);
}

/* Publish to a channel */
void USocketClusterClient::Publish(const FString& channel, const FString& data)
{
	USocketClusterJsonObject* SocketClusterJsonObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonObject->SetStringField(FString(TEXT("event")), FString(TEXT("#publish")));

	USocketClusterJsonObject* SocketClusterJsonDataObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonDataObject->SetStringField("channel", channel);
	SocketClusterJsonDataObject->SetStringField("data", data);

	SocketClusterJsonObject->SetObjectField("data", SocketClusterJsonDataObject);

	Send(SocketClusterJsonObject);
}

void USocketClusterClient::Close(int32 code, USocketClusterJsonObject* data)
{

	USocketClusterJsonObject* SocketClusterJsonObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonObject->SetStringField(FString(TEXT("event")), FString(TEXT("#disconnect")));

	USocketClusterJsonObject* SocketClusterJsonDataObject = NewObject<USocketClusterJsonObject>();
	SocketClusterJsonDataObject->SetNumberField("code", code);
	SocketClusterJsonDataObject->SetObjectField("data", data);

	SocketClusterJsonObject->SetObjectField("data", SocketClusterJsonDataObject);

	UE_LOG(LogSocketClusterClient, Error, TEXT("Disconnect Called : %s"), *SocketClusterJsonObject->EncodeJson());

	Send(SocketClusterJsonObject);
}

void USocketClusterClient::Disconnect()
{
	if (this->state == ESocketClusterState::OPEN || this->state == ESocketClusterState::CONNECTING)
	{
		Close(1000, NULL);
	}
	else
	{
		this->pendingReconnect = false;
		this->pendingReconnectTimeout = NULL;
	}
	
}


void USocketClusterClient::Send(USocketClusterJsonObject* data)
{
	if (this->state != ESocketClusterState::OPEN)
	{
		// close the connection with error code 1005
	}
	else
	{
		FString result = this->codec->Encode(data);
		if (!result.IsEmpty())
		{
			UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Added To Buffer : %s"), *SCC_FUNC_LINE, *result);
			this->Buffer.Add(result);
		}
	}
}