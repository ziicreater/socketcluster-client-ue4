// Modifications Copyright 2020-current ZiiCreater LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved


// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SCJsonModule.h"
#include "SCJsonObject.h"
#include "SCJsonValue.h"

class FSCJsonModule : public SCJsonModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		// @HACK Force classes to be compiled on shipping build
		USCJsonObject::StaticClass();
		USCJsonValue::StaticClass();
	}

	virtual void ShutdownModule() override
	{

	}
};

IMPLEMENT_MODULE(FSCJsonModule, SCJson)

DEFINE_LOG_CATEGORY(SCJson);
