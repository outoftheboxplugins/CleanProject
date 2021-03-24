// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CleanProject : ModuleRules
{
	public CleanProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetManagerEditor",
                "Core",
                "CoreUObject",
                "EditorStyle",
                "Engine",
                "InputCore",
                "MainFrame",
                "Slate",
                "SlateCore",
                "UnrealEd",
			} );
	}
}
