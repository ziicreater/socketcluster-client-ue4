// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

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
