using UnrealBuildTool;

public class Upgrade : ModuleRules
{
    public Upgrade(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "Engine", "Interact",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject", "Engine", "Slate", "SlateCore", "UMG"
            }
        );
    }
}
