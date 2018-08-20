// Fill out your copyright notice in the Description page of Project Settings.

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

	// Blueprint Connect Function
	UFUNCTION(BlueprintCallable, Category = "Socket Cluster")
		static USocketClusterClient* Connect(const FString& url);
	
	
};
