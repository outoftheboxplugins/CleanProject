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
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("MenuSection", "Plugins"));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanupUnusedAssets", "Cleanup unused assets"),
		LOCTEXT("CleanupUnusedAssetsTooltip", "Check dependencies based on all your maps."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				auto Settings = GetDefault<UCleanProjectGameSettings>();
				bool checkMaps = Settings->bCheckAllMapsRefernece;

				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets* with CheckMaps: %s"), (checkMaps ? "enabled" : "disabled"));

				TArray<FAssetData> MapAssetDatas = checkMaps ? CPOperations::GetAllMapAssets() : TArray<FAssetData>();
				CPOperations::CheckDependenciesBasedOn(MapAssetDatas);
			})

		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanupRedirects", "Cleanup redirects"),
		LOCTEXT("CleanupRedirectsTooltip", "Fix redirects in your whole project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Redirects*"));

				CPOperations::FixUpRedirectorsInProject();
			})
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanupEmptyFolders", "Cleanup empty folders"),
		LOCTEXT("CleanupEmptyFoldersTooltip", "Delete all the empty folders from your project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders*"));

				CPOperations::DeleteEmptyProjectFolders();
			})
		));

	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE

