// Modifications Copyright 2020-current ZiiCreater LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved


// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SCJson : ModuleRules
	{
		public SCJson(ReadOnlyTargetRules Target) : base(Target)
        {
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateIncludePaths.AddRange(
				new string[] {
					"ThirdParty/SCJson/Private",
					// ... add other private include paths required here ...
				});

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
                    "HTTP",
                    "Json",
                    "JsonUtilities",
					// ... add other public dependencies that you statically link with here ...
				});
		}
	}
}