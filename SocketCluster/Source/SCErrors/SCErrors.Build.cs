// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

using UnrealBuildTool;

public class SCErrors : ModuleRules
{
	public SCErrors(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "Json",
                "JsonUtilities",
                "SCJson",
			}
			);
	}
}
