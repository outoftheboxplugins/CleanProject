// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CPMenuExtensions.h"

#include "Framework/MultiBox/MultiBoxExtender.h" // for FExtender

#define LOCTEXT_NAMESPACE "CleanProject"

TSharedPtr<FExtender> CPMenuExtensions::CreateMenuExtender()
{
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

	MenuExtender->AddMenuExtension("FileLoadAndSave", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateStatic(CPMenuExtensions::AddMenuExtension));

	return MenuExtender;
}

void CPMenuExtensions::AddMenuExtension(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("CleanProjectSection", "Plugins"));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProjectMaiMenu", "Cleanup unused assets"),
		LOCTEXT("CleanProjectMainMenuTooltip", "Check depedencies based on all your maps."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				auto Settings = GetDefault<UCleanProjectGameSettings>();
				bool checkMaps = Settings->bCheckAllMapsRefernece;

				TArray<FAssetData> MapAssetDatas = checkMaps ? CPOperations::GetAllMapAssets() : TArray<FAssetData>();
				CPOperations::CheckDependenciesBasedOn(MapAssetDatas);
			})

		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProjectMaiMenuRedirects", "Cleanup Redirects"),
		LOCTEXT("CleanProjectMainMenuRedirectsTooltip", "Fix redirects in your whole project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				CPOperations::FixUpRedirectorsInProject();
			})
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProjectMaiMenuFolders", "Cleanup empty folders"),
		LOCTEXT("CleanProjectMainMenuFolderstip", "Delete all the empty folders from your project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				CPOperations::DeleteEmptyProjectFolders();
			})
		));

	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE

