// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

using UnrealBuildTool;

public class CleanProject : ModuleRules
{
	public CleanProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new[]{
			"AssetManagerEditor",
			"AssetTools",
			"ContentBrowser",
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"DeveloperToolSettings",
			"EditorScriptingUtilities",
			"EditorStyle",
			"EditorSubsystem",
			"Engine",
			"Projects",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd",
			"WorkspaceMenuStructure", 
		});
	}
}
