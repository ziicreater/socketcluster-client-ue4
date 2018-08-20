// Fill out your copyright notice in the Description page of Project Settings.

#include "SocketClusterContext.h"
#include "SocketClusterClient.h"
#include "SocketClusterModule.h"

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

extern TSharedPtr<USocketClusterContext> _SocketClusterContext;

// Initialize SocketClusterContext Class.
USocketClusterContext::USocketClusterContext()
{

	protocols = new lws_protocols[3];
	FMemory::Memzero(protocols, sizeof(lws_protocols) * 3);

	protocols[0].name = "websocket";
	protocols[0].callback = USocketClusterContext::ws_service_callback;
	protocols[0].per_session_data_size = 64 * 1024;
	protocols[0].rx_buffer_size = 0;

	lws_context = nullptr;

}

// Create Context Info
void USocketClusterContext::CreateContext()
{

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	info.protocols = &protocols[0];
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;

	info.port = -1;
	info.gid = -1;
	info.uid = -1;

	lws_context = lws_create_context(&info);
	check(lws_context);

}

// Override BeginDestroy Event.
void USocketClusterContext::BeginDestroy()
{
	Super::BeginDestroy();
	_SocketClusterContext.Reset();
}

// Override Tick Event.
void USocketClusterContext::Tick(float DeltaTime)
{
	if (lws_context != nullptr)
	{
		lws_callback_on_writable_all_protocol(lws_context, &protocols[0]);
		lws_service(lws_context, 0);
	}
}

// Override IsTickable Event.
bool USocketClusterContext::IsTickable() const
{
	// We set Tickable to true to make sure this object is created with tickable enabled.
	return true;
}

// Override GetStatId Event.
TStatId USocketClusterContext::GetStatId() const
{
	return TStatId();
}

// Create a new SocketClusterClient using the Connect event.
USocketClusterClient * USocketClusterContext::Connect(const FString & url)
{

	// Check if the context has been created
	if (lws_context == nullptr)
	{
		return nullptr;
	}

	// We create a new SocketClusterClient instance and set the context
	// this makes it possible to have multiple connections to different socketcluster servers
	// without interfering each other
	USocketClusterClient* NewSocketClusterClient = NewObject<USocketClusterClient>();
	NewSocketClusterClient->lws_context = lws_context;
	NewSocketClusterClient->Connect(url);
	return NewSocketClusterClient;
}

// WebSocket Service Callback Function
int USocketClusterContext::ws_service_callback(lws * wsi, lws_callback_reasons reason, void * user, void * in, size_t len)
{
	return 0;
}

