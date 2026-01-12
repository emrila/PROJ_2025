using UnrealBuildTool;

public class Selection : ModuleRules
{
    public Selection(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "GameplayTags",
                "GameplayAbilities",
                "Interact",
                "GameplayUtils"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "Slate",
                "SlateCore",

                "GameplayTasks",
                "NetCore"
            }
        );
    }
}
