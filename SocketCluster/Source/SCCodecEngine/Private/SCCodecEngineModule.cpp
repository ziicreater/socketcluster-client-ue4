// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCCodecEngineModule.h"

#define LOCTEXT_NAMESPACE "FSCCodecEngineModule"

DEFINE_LOG_CATEGORY(SCCodecEngine);

void FSCCodecEngineModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSCCodecEngineModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSCCodecEngineModule, SCCodecEngine)