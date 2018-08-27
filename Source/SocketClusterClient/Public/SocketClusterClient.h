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
#include "SocketClusterModule.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "Runtime/JsonUtilities/Public/JsonObjectWrapper.h"
#include "Delegates/DelegateCombinations.h"
#include "LatentActions.h"
#include "SocketClusterClient.generated.h"

typedef struct lws sc_lws;
typedef struct lws_context sc_lws_context;
typedef struct lws_set_wsi_user sc_lws_set_wsi_user;

DECLARE_DYNAMIC_DELEGATE_OneParam(FResponseCallback, FString, Response);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SOCKETCLUSTERCLIENT_API USocketClusterClient : public UObject
{
	GENERATED_BODY()

public:

	// Initialize SocketClusterClient Class.
	USocketClusterClient();
	
	// Override BeginDestroy event.
	virtual void BeginDestroy() override;

	// Connect To SocketCluster Server Function.
	void Connect(const FString& url);

	// Disconnect From SocketCluster Server Function.
	void Disconnect();

	// Emit To SocketCluster Server Function.
	void Emit(UObject* WorldContextObject, const FString& event, const FString& data, const FResponseCallback& callback, struct FLatentActionInfo LatentInfo);

	void WriteBuffer();

	// Call ID
	double cid;

	// AckTimeOut
	static float AckTimeout;

	sc_lws* lws;
	sc_lws_context* lws_context;
	sc_lws_set_wsi_user* lws_set_wsi_user;

	// Responses Buffer
	static TMap<double, FString> Responses;

	// Send Buffer
	TArray<TSharedPtr<FJsonObject>> Send;

};
