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

#include "SocketClusterClient.h"
#include "SocketClusterModule.h"
#include "SocketClusterContext.h"

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

// Initialize SocketClusterClient Class.
USocketClusterClient::USocketClusterClient()
{
	lws_context = nullptr;
	lws = nullptr;
}

// Override BeginDestroy Event.
void USocketClusterClient::BeginDestroy()
{
	Super::BeginDestroy();
}

// Connect To SocketCluster Server Function.
void USocketClusterClient::Connect(const FString & url)
{

	// Exit if the url passed is empty.
	if (url.IsEmpty())
	{
		return;
	}

	// Check if the SocketClusterContext has already been created other wise exit
	if (lws_context == nullptr)
	{
		return;
	}

	// By default we don't connect using SSL
	int UseSSL = 0;

	// Check if we passed the right url format
	int url_find = url.Find(TEXT(":"));
	if (url_find == INDEX_NONE)
	{
		return;
	}

	// Get the protocol we wan't connect with
	FString protocol = url.Left(url_find);

	// Check if we passed a supported protocol
	if (protocol.ToLower() != TEXT("ws") && protocol.ToLower() != TEXT("wss"))
	{
		return;
	}

	// Check if we wan't to connect using SSL
	if (protocol.ToLower() == TEXT("wss"))
	{
		UseSSL = 2;
	}

	FString host;
	FString path = TEXT("/");
	FString next_part = url.Mid(url_find + 3);
	url_find = next_part.Find("/");

	// Check if we passed a path other then /
	// example : ws://example.com/mypath/
	if (url_find != INDEX_NONE)
	{
		host = next_part.Left(url_find);
		path = next_part.Mid(url_find);
	}
	else
	{
		host = next_part;
	}

	// Set address to current host
	FString address = host;

	// Set default port to 80
	int port = 80;

	url_find = address.Find(":");

	// Check if we passed a port to the url
	if (url_find != INDEX_NONE)
	{
		address = host.Left(url_find);
		port = FCString::Atoi(*host.Mid(url_find + 1));
	}
	else
	{
		// If we diden't pass a port and we are using SSL set port to 443
		if (UseSSL)
		{
			port = 443;
		}
	}

	// Create the connection info
	struct lws_client_connect_info info;
	memset(&info, 0, sizeof(info));

	// Convert the FString type to std:string so libwebsockets can read it.
	std::string stdaddress = TCHAR_TO_UTF8(*address);
	std::string stdpath = TCHAR_TO_UTF8(*path);
	std::string stdhost = TCHAR_TO_UTF8(*host);

	// Set connection info
	info.context = lws_context;
	info.address = stdaddress.c_str();
	info.port = port;
	info.ssl_connection = UseSSL;
	info.path = stdpath.c_str();
	info.host = stdhost.c_str();
	info.origin = stdhost.c_str();
	info.ietf_version_or_minus_one = -1;
	info.userdata = this;

	// Create connection
	lws = lws_client_connect_via_info(&info);

	// Check if creating connection info was successful
	if (lws == nullptr)
	{
		UE_LOG(SocketClusterClientLog, Error, TEXT("Error Trying To Create Client Connecton."));
		return;
	}
}

// Disconnect From SocketCluster Server Function.
void USocketClusterClient::Disconnect()
{
	if (lws != nullptr)
	{
		// Call Disconnect On SocketClusterContext
		_SocketClusterContext->Disconnect();
	}
}
