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

#include "SocketCluster.h"
#include "SocketClusterContext.h"

TSharedPtr<USocketClusterContext> _SocketClusterContext;

// Blueprint Connect Function
USocketClusterClient* USocketCluster::Connect(const FString& url, const float ackTimeout)
{
	// Check if the shared SocketClusterContext already exists
	if (_SocketClusterContext.Get() == nullptr)
	{
		// Create a new SocketClusterContext object and make it shareable
		_SocketClusterContext = MakeShareable(NewObject<USocketClusterContext>());

		// Call CreateContext Function on SocketClusterContext
		_SocketClusterContext->CreateContext();

		// Add the new SocketClusterContext to Root GC list
		_SocketClusterContext->AddToRoot();
	}

	// Return SocketClusterContext Connect Function
	return _SocketClusterContext->Connect(url, ackTimeout);
}

// Blueprint Disconnect Function
void USocketCluster::Disconnect(USocketClusterClient * SocketClusterClient)
{
	// Check if the SocketClusterClient still exists before calling a function
	if (SocketClusterClient != nullptr)
	{
		// Call Disconnect Function On the given SocketClusterClient
		SocketClusterClient->Disconnect();
	}
}

void USocketCluster::Emit(USocketClusterClient* SocketClusterClient, UObject* WorldContextObject, const FString& event, const FString& data, const FResponseCallback& callback, FLatentActionInfo LatentInfo)
{
	// Check if the SocketClusterClient still exists before calling a function
	if (SocketClusterClient != nullptr)
	{
		SocketClusterClient->Emit(WorldContextObject, event, data, callback, LatentInfo);
	}
}
