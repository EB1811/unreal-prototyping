// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Prototyping : ModuleRules
{
	public Prototyping(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "SlateCore", "Slate", "InputCore", "EnhancedInput", "UMG", "Paper2D", "AIModule", "NavigationSystem", "GameplayTags", "Landscape", "DeveloperSettings" }); 

		PrivateDependencyModuleNames.AddRange(new string[] { "MassEntity", "MassCommon", "MassMovement", "Renderer", "RenderCore", "RHI", "CommonUI", "MediaAssets", "PhysicsCore", "OnlineSubsystem"});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
