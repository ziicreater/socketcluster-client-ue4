// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

#include "SCJsonModule.h"

#define LOCTEXT_NAMESPACE "FSCJsonModule"

DEFINE_LOG_CATEGORY(LogSCJson);

void FSCJsonModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSCJsonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSCJsonModule, SCJson)