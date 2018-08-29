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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SocketClusterClient.h"
#include "SocketCluster.generated.h"

/**
 * 
 */
UCLASS()
class SOCKETCLUSTERCLIENT_API USocketCluster : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/* Returns the state of the socket */
	UFUNCTION(BlueprintPure, Category = "Socket Cluster")
		static EState GetState(USocketClusterClient* SocketClusterClient);

	/* Returns the auth state of the socket */
	UFUNCTION(BlueprintPure, Category = "Socket Cluster")
		static EAuthState GetAuthState(USocketClusterClient* SocketClusterClient);

	/* Returns the auth token associated with the socket */
	UFUNCTION(BlueprintPure, Category = "Socket Cluster")
		static FString GetAuthToken(USocketClusterClient* SocketClusterClient);

	/* Returns the signed auth token associated with the socket */
	UFUNCTION(BlueprintPure, Category = "Socket Cluster")
		static FString GetSignedAuthToken(USocketClusterClient* SocketClusterClient);

	/* Connect to SocketCluster Server */
	UFUNCTION(BlueprintCallable, Category = "Socket Cluster", meta = (AdvancedDisplay = "1"))
		static USocketClusterClient* Connect(const FString& url, const float ackTimeout = 10.0f);

	/* Disconnect From SocketCluster Server */
	UFUNCTION(BlueprintCallable, Category = "Socket Cluster")
		static void Disconnect(USocketClusterClient* SocketClusterClient);

	/* Send Emit To SocketCluster Server */
	UFUNCTION(BlueprintCallable, Category = "Socket Cluster", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", AutoCreateRefTerm = "callback", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "4"))
		static void Emit(USocketClusterClient* SocketClusterClient, UObject* WorldContextObject, const FString& event, const FString& data, const FResponseCallback& callback, struct FLatentActionInfo LatentInfo);
	
	
};
