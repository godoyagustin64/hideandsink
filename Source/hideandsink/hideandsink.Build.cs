// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class hideandsink : ModuleRules
{
	public hideandsink(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"hideandsink",
			"hideandsink/Variant_Platforming",
			"hideandsink/Variant_Platforming/Animation",
			"hideandsink/Variant_Combat",
			"hideandsink/Variant_Combat/AI",
			"hideandsink/Variant_Combat/Animation",
			"hideandsink/Variant_Combat/Gameplay",
			"hideandsink/Variant_Combat/Interfaces",
			"hideandsink/Variant_Combat/UI",
			"hideandsink/Variant_SideScrolling",
			"hideandsink/Variant_SideScrolling/AI",
			"hideandsink/Variant_SideScrolling/Gameplay",
			"hideandsink/Variant_SideScrolling/Interfaces",
			"hideandsink/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
