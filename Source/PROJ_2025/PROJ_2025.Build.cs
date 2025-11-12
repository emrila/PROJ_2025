// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PROJ_2025 : ModuleRules
{
	public PROJ_2025(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities",
			"HTTP",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"UMG",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Upgrade" });

		PublicIncludePaths.AddRange(new string[] {
			"PROJ_2025",
			"PROJ_2025/Variant_Platforming",
			"PROJ_2025/Variant_Platforming/Animation",
			"PROJ_2025/Variant_Combat",
			"PROJ_2025/Variant_Combat/AI",
			"PROJ_2025/Variant_Combat/Animation",
			"PROJ_2025/Variant_Combat/Gameplay",
			"PROJ_2025/Variant_Combat/Interfaces",
			"PROJ_2025/Variant_Combat/UI",
			"PROJ_2025/Variant_SideScrolling",
			"PROJ_2025/Variant_SideScrolling/AI",
			"PROJ_2025/Variant_SideScrolling/Gameplay",
			"PROJ_2025/Variant_SideScrolling/Interfaces",
			"PROJ_2025/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
