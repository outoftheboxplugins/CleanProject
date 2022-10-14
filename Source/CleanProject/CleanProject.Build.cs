// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

using UnrealBuildTool;

public class CleanProject : ModuleRules
{
	public CleanProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetTools",
				"AssetManagerEditor",
				"Core",
				"CoreUObject",
				"ContentBrowser",
				"DeveloperSettings",
				"DeveloperToolSettings",
				"EditorSubsystem",
				"EditorStyle",
				"Engine",
				"Projects",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"UnrealEd",
				"WorkspaceMenuStructure", 
				"EditorScriptingUtilities",
			} );
	}
}
