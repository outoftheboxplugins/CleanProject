// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

using UnrealBuildTool;

public class CleanProject : ModuleRules
{
	public CleanProject(ReadOnlyTargetRules Target) : base(Target)
	{
		//TODO: Check if all of these are needed
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetManagerEditor",
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"DeveloperToolSettings",
				"EditorSubsystem",
				"EditorStyle",
				"Engine",
				"InputCore",
				"MainFrame",
				"Projects",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"UnrealEd",
				"WorkspaceMenuStructure",
			} );
	}
}
