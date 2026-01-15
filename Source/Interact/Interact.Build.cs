using UnrealBuildTool;

public class Interact : ModuleRules
{
    public Interact(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
                "GameplayUtils",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Slate",
                "SlateCore"
            }
        );
    }
}
