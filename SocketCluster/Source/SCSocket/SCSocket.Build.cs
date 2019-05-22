// Copyright 2019 ZiiCreater, LLC. All Rights Reserved.

using UnrealBuildTool;

public class SCSocket : ModuleRules
{
	public SCSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
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
                "SCErrors",
				"libWebSockets",
				"OpenSSL",
				"zlib",
				// ... add private dependencies that you statically link with here ...	
			}
			);
	}
}
