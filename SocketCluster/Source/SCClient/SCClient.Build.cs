// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SCClient : ModuleRules
{
	public SCClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Json",
                "SCSocket",
                "SCCodecEngine",
                "SCAuthEngine",
                "SCErrors",
                "SCJson",
			}
           );

    }
}
