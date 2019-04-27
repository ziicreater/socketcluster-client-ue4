// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SCJsonConvert.h"
#include "SCJsonObject.h"
#include "SCJsonValue.h"
#include "SCResponse.generated.h"

class USCTransport;

/**
* The SocketCluster Response
*/
UCLASS(Blueprintable, BlueprintType, DisplayName = "SCResponse")
class SCCLIENT_API USCResponse : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	USCTransport* socket;

	int32 id;

	bool sent;
	
public:

	void create(USCTransport* transport, int32 cid);

	void _respond(TSharedPtr<FJsonValue> responseData);

	void end(TSharedPtr<FJsonValue> data = nullptr);

	void error(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data = nullptr);

	void callback(TSharedPtr<FJsonValue> error, TSharedPtr<FJsonValue> data);

	void res(TSharedPtr<FJsonValue> error = nullptr, TSharedPtr<FJsonValue> data = nullptr);

	/** Send a response back to the server */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Res", AutoCreateRefTerm = "error, data"), Category = "SocketCluster|Client")
		void resBlueprint(USCJsonValue* error, USCJsonValue* data);
		
};
