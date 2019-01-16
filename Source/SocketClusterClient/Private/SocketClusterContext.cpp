// Copyright 2018 ZiiCreater, LLC. All Rights Reserved.

#include "SocketClusterContext.h"
#include "SocketClusterClient.h"
#include "SC_Formatter.h"
#include "SocketClusterClientModule.h"
#include "SocketClusterJsonObject.h"

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

static struct lws_protocols protocols[] = {
	{
		"SocketCluster",
		USocketClusterContext::ws_service_callback,
		0,
	},
	{
		NULL,
		NULL,
		0
	}
};

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{
		NULL,
		NULL,
		NULL
	}
};

USocketClusterContext::USocketClusterContext()
{
	context = nullptr;
}

void USocketClusterContext::BeginDestroy()
{
	if (context != nullptr)
	{
		lws_context_destroy(context);
		context = nullptr;
	}
	Super::BeginDestroy();
}

void USocketClusterContext::Tick(float DeltaTime)
{
	if (context != nullptr)
	{
		lws_callback_on_writable_all_protocol(context, &protocols[0]);
		lws_service(context, 0);
	}
}

bool USocketClusterContext::IsTickable() const
{
	return true;
}

TStatId USocketClusterContext::GetStatId() const
{
	return TStatId();
}

/* Receive from server function */
int USocketClusterContext::ws_service_callback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
{
	void* wsi_user = lws_wsi_user(wsi);
	USocketClusterClient* SocketClusterClient = (USocketClusterClient*)wsi_user;

	switch (reason)
	{
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			SocketClusterClient->state = ESocketClusterState::OPEN;
			//UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Connected"), *SCC_FUNC_LINE);

			// Send handshake to the server
			USocketClusterJsonObject* SocketClusterJsonObject = NewObject<USocketClusterJsonObject>();
			SocketClusterJsonObject->SetStringField(FString(TEXT("event")), FString(TEXT("#handshake")));
			SocketClusterJsonObject->SetIntegerField(FString(TEXT("cid")), SocketClusterClient->callIdGenerator());
			SocketClusterJsonObject->SetObjectField(FString(TEXT("data")), NULL);
			SocketClusterClient->Send(SocketClusterJsonObject);

			// Call OnConnect event 
			SocketClusterClient->OnConnect.Broadcast();
		}
		break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
		{
			if (!SocketClusterClient) return -1;

			FString result = UTF8_TO_TCHAR(in);
			if (result.Equals("#1"))
			{
				UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Heart Beat"), *SCC_FUNC_LINE);
				ws_write_back(wsi, (char*)"#2", -1);
			}
			else
			{

				USocketClusterJsonObject* SocketClusterJsonObject = SocketClusterClient->codec->Decode(result);
				if (SocketClusterJsonObject != nullptr)
				{  

					if (SocketClusterJsonObject->HasField("data") && SocketClusterJsonObject->GetObjectField("data"))
					{
						USocketClusterJsonObject* data = SocketClusterJsonObject->GetObjectField("data");

						if (data->HasField("channel"))
						{
							FString channel = data->GetStringField("channel");
						}
						if (data->HasField("token"))
						{
							SocketClusterClient->authToken = data->GetStringField("token");
						}
						if (data->HasField("isAuthenticated"))
						{
							bool isAuthenticated = data->GetBoolField("isAuthenticated");
						}
						if (data->HasField("id"))
						{
							SocketClusterClient->id = data->GetStringField("id");
						}
						if (data->HasField("pingTimeout"))
						{
							SocketClusterClient->pingTimeout = (data->GetNumberField("pingTimeout") / 1000);
						}
						UE_LOG(LogSocketClusterClient, Error, TEXT("%s : We have a data object"), *SCC_FUNC_LINE);
					}

					if (SocketClusterJsonObject->HasField("event"))
					{
						FString event = SocketClusterJsonObject->GetStringField("event");
						USocketClusterJsonObject* data = SocketClusterJsonObject->GetObjectField("data");

						if (event.Equals("#publish"))
						{
							SocketClusterClient->OnPublishCallback.ExecuteIfBound(data->GetStringField("channel"), data->GetStringField("data"));
						}
						else if (event.Equals("#removeAuthToken"))
						{
							SocketClusterClient->authToken = NULL;
						}
						else if (event.Equals("#setAuthToken"))
						{
							//SocketClusterClient->OnAuthTokenCallback.ExecuteIfBound(SocketClusterClient->authToken);
						}
						else
						{
							// Events
						}
					}
					else
					{
						int32 rid = SocketClusterJsonObject->GetIntegerField("rid");
						if (rid == 1)
						{
							//isAuthenticated
						}
					}
					
				}
				UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Received %s"), *SCC_FUNC_LINE, *result);
			}
		}
		break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		{
			if (!SocketClusterClient) return -1;
			FString strError = UTF8_TO_TCHAR(in);
			//Error Event.
		}
		break;

		case LWS_CALLBACK_CLOSED:
		{
			if (!SocketClusterClient) return -1;
			SocketClusterClient->OnDisconnect.Broadcast();
		}
		break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
			if (!SocketClusterClient) return -1;
			while (SocketClusterClient->Buffer.Num() > 0)
			{
				std::string strData = TCHAR_TO_UTF8(*SocketClusterClient->Buffer[0]);
				ws_write_back(wsi, strData.c_str(), strData.size());
				SocketClusterClient->Buffer.RemoveAt(0);
			}
		}
		break;

		default:
			break;
	}

	return 0;
}

/* Send to server function */
int USocketClusterContext::ws_write_back(lws* lws, const char* str, int str_size_in)
{
	if (str == NULL || lws == NULL)
		return -1;

	int n;
	int len;
	unsigned char *out = NULL;

	if (str_size_in < 1)
		len = strlen(str);
	else
		len = str_size_in;

	out = (unsigned char*)malloc(sizeof(unsigned char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));

	memcpy(out + LWS_SEND_BUFFER_PRE_PADDING, str, len);

	n = lws_write(lws, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	free(out);

	return n;
}

USocketClusterClient* USocketClusterContext::Create
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

	/* Create new instance of Socket Cluster Client */
	USocketClusterClient* SocketClusterClient = NewObject<USocketClusterClient>();

	/* Set the variables for the Socket Cluster Client */
	SocketClusterClient->uri = uri;
	SocketClusterClient->bAutoReconnect = AutoReconnect;
	SocketClusterClient->InitialDelay = ReconnectInitialDelay;
	SocketClusterClient->Randomness = ReconnectRandomness;
	SocketClusterClient->Multiplier = ReconnectMultiplier;
	SocketClusterClient->MaxDelay = ReconnectMaxDelay;
	SocketClusterClient->bAutoSubscribeOnConnect = AutoSubscribeOnConnect;
	SocketClusterClient->connectTimeout = ConnectTimeout;
	SocketClusterClient->ackTimeout = AckTimeout;
	SocketClusterClient->bTimestampRequests = TimestampRequests;
	SocketClusterClient->timestampParam = TimestampParam;
	SocketClusterClient->query = Query;

	/* Set the current pingTimeout of the socket to ackTimeout it will be updated with values provided by the 'connect' event */
	SocketClusterClient->pingTimeout = AckTimeout;

	/* Set the socket context */
	SocketClusterClient->context = this->context;

	/* Check if we passed a Codec */
	if (Codec == nullptr)
	{
		/* Set the default codecEngine (SC_Formatter) */
		USC_Formatter* SC_Formatter = NewObject<USC_Formatter>();
		Codec = SC_Formatter;
	}

	/* Check if the Codec we requested is a valid type */
	ISocketClusterCodecEngine* CodecEngineObject = Cast<ISocketClusterCodecEngine>(Codec);
	if (!CodecEngineObject)
	{
		FString CodecEngineObjectName = Codec->GetName();
		UE_LOG(LogSocketClusterClient, Error, TEXT("%s : %s must implement ISocketClusterCodecEngine"), *SCC_FUNC_LINE, *CodecEngineObjectName);
		return nullptr;
	}
	else
	{
		/* Set the codecEngine on SocketClusterClient Instance */
		SocketClusterClient->codec = CodecEngineObject;
	}

	/* Check if we should autoconnect */
	if (AutoConnect)
	{
		SocketClusterClient->Connect();
	}
	
	return SocketClusterClient;
}

bool USocketClusterContext::CreateContext()
{
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	info.protocols = protocols;
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.gid = -1;
	info.uid = -1;
	info.extensions = exts;
	info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;

#if ENGINE_MINOR_VERSION >= 20
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

	context = lws_create_context(&info);
	if (context == nullptr)
	{
		UE_LOG(LogSocketClusterClient, Error, TEXT("%s : Failed To Create Context"), *SCC_FUNC_LINE);
		return false;
	}

	return true;
}
