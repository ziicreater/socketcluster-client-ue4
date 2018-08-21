/*
*
*	MIT License
*
*	Copyright(c) 2018 ZiiCreater LLC
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files(the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions :
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/

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

// Disconnect from current SocketClusterContext connection.
void USocketClusterContext::Disconnect()
{
	// Make sure we trying to disconnect on a connected SocketClusterContext
	if (lws_context != nullptr)
	{
		// Call LWS Destroy Event
		lws_context_destroy(lws_context);

		// Set our Context to a nullptr 
		lws_context = nullptr;

		// Delete the current Protocols
		delete protocols;

		// Set our Protocols to a nullptr
		protocols = nullptr;
	}
}

// WebSocket Service Callback Function
int USocketClusterContext::ws_service_callback(lws * wsi, lws_callback_reasons reason, void * user, void * in, size_t len)
{
	return 0;
}

