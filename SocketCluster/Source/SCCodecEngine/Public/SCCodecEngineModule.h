// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSCCodecEngine, Log, All);

class FSCCodecEngineModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
