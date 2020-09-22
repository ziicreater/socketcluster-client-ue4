// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCAuthEngine.generated.h"

/**
 * SocketCluster Default AuthEngine Interface
 * 
 * Saves the AuthToken in a Memory Key/Value Storage.
 * 
 * @SaveToken
 * Saves the token to the memory key/value storage using the name as the key
 * 
 * @RemoveToken
 * Removes the token from the memory key/value storage using the name as the key
 * 
 * @LoadToken
 * Loads the token from the memory key/value storage using the name as the key
 * 
 */
UCLASS(BlueprintType, Abstract)
class SCAUTHENGINE_API USCAuthEngine : public UObject
{
	GENERATED_BODY()

protected:

	/**
	* In memory Key/Value storage
	*/
	TMap<FString, FString> localStorage;

public:
	
	/**
	* Save Authentication Token.
	*
	* @param Name		Authentication Token Name
	* @param Token		Authentication Token Value
	* 
	* @return			Returns Token that has been saved.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SaveToken"), Category = "SocketCluster|AuthEngine")
	virtual FString SaveToken(FString Name, FString Token);

	/**
	* Remove Authentication Token.
	*
	* @param Name		Authentication Token Name
	*
	* @return			Returns Token that has been removed.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RemoveToken"), Category = "SocketCluster|AuthEngine")
	virtual FString RemoveToken(FString Name);

	/**
	* Load Authentication Token.
	*
	* @param Name		Authentication Token Name
	*
	* @return			Returns Token.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadToken"), Category = "SocketCluster|AuthEngine")
	virtual FString LoadToken(FString Name);

};
