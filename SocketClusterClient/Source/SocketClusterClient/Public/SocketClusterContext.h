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

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Tickable.h"
#include "SocketClusterContext.generated.h"

typedef struct lws_context sc_lws_context;
typedef struct lws_protocols sc_lws_protocols;

class USocketClusterClient;

/**
 * 
 */
UCLASS()
class SOCKETCLUSTERCLIENT_API USocketClusterContext : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:

	// Initialize SocketClusterContext Class.
	USocketClusterContext();

	// Create Context Info
	void CreateContext();

	// Override BeginDestroy Event.
	virtual void BeginDestroy() override;

	// Override Tick Event.
	virtual void Tick(float DeltaTime) override;

	// Override IsTickable Event.
	virtual bool IsTickable() const override;

	// Override GetStatId Event.
	virtual TStatId GetStatId() const override;

	// Create a new SocketClusterClient using the Connect event.
	USocketClusterClient* Connect(const FString& url);

	// WebSocket Service Callback Function
	static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

	sc_lws_context* lws_context;
	sc_lws_protocols* protocols;
	
};
