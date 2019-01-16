// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSocketClusterClient, Log, All);

#define SCC_FUNC (FString(__FUNCTION__))
#define SCC_LINE (FString::FromInt(__LINE__))
#define SCC_FUNC_LINE (SCC_FUNC + "(" + SCC_LINE + ")")

class FSocketClusterClientModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
