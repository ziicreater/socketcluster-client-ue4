// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSCClient, Log, All);

#define SCC_FUNC (FString(__FUNCTION__))
#define SCC_LINE (FString::FromInt(__LINE__))
#define SCC_FUNC_LINE (SCC_FUNC + "(" + SCC_LINE + ")")

class FSCClientModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
