// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
using UnrealBuildTool;

public class CleanProject : ModuleRules
{
	public CleanProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "EditorStyle",
                "Engine",
                "InputCore",
                "MainFrame",
                "SlateCore",
                "UnrealEd",
				"AssetManagerEditor",
				"Slate",
				"WorkspaceMenuStructure",
			} );
	}
}
