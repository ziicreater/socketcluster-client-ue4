// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSCJson, Log, All);

#define SCJ_FUNC (FString(__FUNCTION__))
#define SCJ_LINE (FString::FromInt(__LINE__))
#define SCJ_FUNC_LINE (SCJ_FUNC + "(" + SCJ_LINE + ")")

class FSCJsonModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
