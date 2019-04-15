// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonObject.h"
#include "SCResponse.generated.h"

class USCTransport;

/**
* The SocketCluster Response
*/
UCLASS()
class SCCLIENT_API USCResponse : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	USCTransport* socket;

	int32 id;

	bool sent;
	
public:

	void create(USCTransport* transport, int32 cid);

	void _respond(USCJsonObject* responseData);

	void end(USCJsonObject* data = nullptr);

	void error(USCJsonObject* error, USCJsonObject* data = nullptr);

	void callback(USCJsonObject* error, USCJsonObject* data);

	/** Send a response back to the server */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Res", AutoCreateRefTerm = "error, data"), Category = "SocketCluster|Client")
		void res(USCJsonObject* error, USCJsonObject* data);
		
};
