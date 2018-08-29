// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SocketClusterClient : ModuleRules
{

#if WITH_FORWARDED_MODULE_RULES_CTOR
    public SocketClusterClient(ReadOnlyTargetRules Target) : base(Target) // > 4.15
#else
    public SocketClusterClient(TargetInfo Target) // < 4.15
#endif
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				"SocketClusterClient/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"SocketClusterClient/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Networking",
                "Sockets",
                "libWebSockets",
                "OpenSSL",
                "zlib",
                "Json",
                "JsonUtilities"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Core",
                "Networking",
                "Sockets",
                "libWebSockets",
                "OpenSSL",
                "zlib",
                "Json",
                "JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
