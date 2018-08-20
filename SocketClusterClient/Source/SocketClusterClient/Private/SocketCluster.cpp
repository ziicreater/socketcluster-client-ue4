// Fill out your copyright notice in the Description page of Project Settings.

#include "SocketCluster.h"
#include "SocketClusterContext.h"

TSharedPtr<USocketClusterContext> _SocketClusterContext;

// Blueprint Connect Function
USocketClusterClient* USocketCluster::Connect(const FString& url)
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
	return _SocketClusterContext->Connect(url);
}




