// Modifications Copyright 2019 ZiiCreater, LLC. All Rights Reserved.
// Modifications Copyright 2018-current Getnamo. All Rights Reserved
// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

using UnrealBuildTool;

public class SCJson : ModuleRules
{
    public SCJson(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
            new string[] {
                "ThirdParty/SCJson/Private",
            });


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "HTTP",
                    "Json",
                    "JsonUtilities"
            });
    }
}
