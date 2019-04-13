// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SCClient : ModuleRules
{
	public SCClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Sockets",
                "libWebSockets",
                "OpenSSL",
                "zlib",
                "Json",
                "SCCodecEngine",
                "SCAuthEngine",
                "SCErrors",
                "SCJson",
			}
           );

    }
}
