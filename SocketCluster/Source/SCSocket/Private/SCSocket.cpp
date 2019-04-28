// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCSocket.h"
#include "Runtime/Launch/Resources/Version.h"
#include "SCErrors.h"
#include "SCSocketModule.h"

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
		"socketcluster",
		USCSocket::ws_service_callback,
		0,
	},
	{
		NULL,
		NULL,
		0
	}
};

const struct lws_protocol_vhost_options pvo_opt = {
	NULL,
	NULL,
	"default",
	"1"
};

const struct lws_protocol_vhost_options pvo = {
	NULL,
	&pvo_opt,
	"socketcluster",
	""
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

static void lws_debug(int level, const char *line)
{
	UE_LOG(LogSCSocket, Log, TEXT("%s"), ANSI_TO_TCHAR(line));
}

void USCSocket::Tick(float DeltaTime)
{
	if (context != nullptr)
	{
		lws_callback_on_writable_all_protocol(context, &protocols[0]);
		lws_service(context, 0);
	}
}

bool USCSocket::IsTickable() const
{
	return true;
}

TStatId USCSocket::GetStatId() const
{
	return TStatId();
}

int USCSocket::ws_service_callback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
{

	void* wsi_user = lws_wsi_user(wsi);
	USCSocket* SCSocket = (USCSocket*)wsi_user;

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
	{
		SCSocket->readyState = ESocketState::OPEN;
		if (SCSocket->onopen)
		{
			SCSocket->onopen();
		}
	}
	break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
	{
		TSharedPtr<FJsonValue> Error = USCErrors::SocketProtocolError(UTF8_TO_TCHAR(in), 1005);
		if (SCSocket->onerror)
		{
			SCSocket->onerror(Error);
		}
	}
	break;
	case LWS_CALLBACK_CLOSED:
	{
		if (SCSocket->onclose)
		{
			TSharedPtr<FJsonObject> Error = MakeShareable(new FJsonObject);
			Error->SetNumberField("code", 1001);
			Error->SetStringField("reason", UTF8_TO_TCHAR(in));
			SCSocket->onclose(Error);
		}
	}
	break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
	{
		FString message = UTF8_TO_TCHAR(in);
		if (SCSocket->onmessage)
		{
			SCSocket->onmessage(message);
		}
	}
	break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:
	{
		if (SCSocket->_buffer.Num() > 0)
		{
			std::string strData = TCHAR_TO_UTF8(*SCSocket->_buffer[0]);
			ws_write_back(wsi, strData.c_str(), strData.size());
			SCSocket->_buffer.RemoveAt(0);
		}
	}
	break;
	}
	return 0;
}

int USCSocket::ws_write_back(lws* wsi, const char* str, int str_size_in)
{
	if (str == NULL || wsi == NULL)
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

	n = lws_write(wsi, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	free(out);

	return n;
}

void USCSocket::createWebSocket(FString uri, TSharedPtr<FJsonObject> options)
{

#if !UE_BUILD_SHIPPING
	lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG | LLL_INFO, lws_debug);
#endif

	readyState = ESocketState::CLOSED;

	struct lws_context_creation_info context_info;
	memset(&context_info, 0, sizeof(context_info));

	context_info.protocols = protocols;
	context_info.pvo = &pvo;
	context_info.ssl_cert_filepath = NULL;
	context_info.ssl_private_key_filepath = NULL;

	context_info.port = CONTEXT_PORT_NO_LISTEN;
	context_info.gid = -1;
	context_info.uid = -1;
	context_info.extensions = exts;
	context_info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;

#if ENGINE_MINOR_VERSION >= 20
	context_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

	context = lws_create_context(&context_info);

	if (!context)
	{
		UE_LOG(LogSCSocket, Error, TEXT("context failed"));
		return;
	}

	int pos = uri.Find(":");
	if (pos == INDEX_NONE)
	{
		if (onerror)
		{
			TSharedPtr<FJsonValue> Error = USCErrors::SocketProtocolError("Invalid URL:" + uri, 0);
			onerror(Error);
		}
		return;
	}

	int ssl = 0;
	FString protocol = uri.Left(pos);
	if (protocol.ToUpper().Equals("WSS") || protocol.ToUpper().Equals("HTTPS"))
	{
		if (options->HasField("rejectUnauthorized") && !options->GetBoolField("rejectUnauthorized"))
		{
			ssl = 2;
		}
		else
		{
			ssl = 1;
		}
	}

	FString host;
	FString path = "/";
	FString next = uri.Mid(pos + 3);
	pos = next.Find("/");
	if (pos != INDEX_NONE)
	{
		host = next.Left(pos);
		path = next.Mid(pos);
	}
	else
	{
		host = next;
	}

	FString address = host;

	int port;
	pos = address.Find(":");
	if (pos != INDEX_NONE)
	{
		address = host.Left(pos);
		port = FCString::Atoi(*host.Mid(pos + 1));
	}
	else
	{
		if (ssl == 1 || ssl == 2)
		{
			port = 443;
		}
		else
		{
			port = 80;
		}
	}

	struct lws_client_connect_info client_info;
	memset(&client_info, 0, sizeof(client_info));

	std::string stdAddress = TCHAR_TO_UTF8(*address);
	std::string stdPath = TCHAR_TO_UTF8(*path);
	std::string stdHost = TCHAR_TO_UTF8(*host);

	client_info.context = context;
	client_info.address = stdAddress.c_str();
	client_info.port = port;
	client_info.ssl_connection = ssl;
	client_info.path = stdPath.c_str();
	client_info.host = stdHost.c_str();
	client_info.origin = stdHost.c_str();
	client_info.ietf_version_or_minus_one = -1;
	client_info.protocol = protocols[0].name;
	client_info.userdata = this;

	socket = lws_client_connect_via_info(&client_info);

	if (!socket)
	{
		UE_LOG(LogSCSocket, Error, TEXT("lws failed"));
		return;
	}
}

void USCSocket::send(FString data)
{
	std::string strData = TCHAR_TO_UTF8(*data);
	ws_write_back(socket, strData.c_str(), strData.size());
}

void USCSocket::sendBuffer(FString data)
{
	_buffer.Add(data);	
}

void USCSocket::close(int32 code)
{

}
