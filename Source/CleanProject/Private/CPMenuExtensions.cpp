// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CPMenuExtensions.h"

#include "CPOperations.h"

#include "Framework/MultiBox/MultiBoxExtender.h" // for FExtender

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const FName MainMenuExtensionHook = FName("FileLoadAndSave");
}

TSharedPtr<FExtender> CPMenuExtensions::CreateMenuExtender()
{
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

	MenuExtender->AddMenuExtension(MainMenuExtensionHook, EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateStatic(CPMenuExtensions::AddMenuExtension));

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
				auto Settings = GetDefault<UCPProjectSettings>();
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

TSharedRef<FExtender> CPMenuExtensions::CreateContentBrowserExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(CPMenuExtensions::CreateContentBrowserEntry, SelectedAssets));

	return ContentBrowserExtender;
}

void CPMenuExtensions::CreateContentBrowserEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.BeginSection(TEXT("CleanProject_ContentBrowserSection"),
		LOCTEXT("CleanProject_ContentBrowserSection", "Clean Project"));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("CleanProject_ContentBrowserBasedTitle", "Check unused assets based on selected"),
		LOCTEXT("CleanProject_ContentBrowserBasedTooltip", "Returns all the assets not used by the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				CPOperations::CheckDependenciesBasedOn(SelectedAssets);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("CleanProject_ContentBrowserForTitle", "Check if selected assets are unused"),
		LOCTEXT("CleanProject_ContentBrowserForTooltip", "Returns the unused assets from the list of selected assets based on the dependencies of all."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				CPOperations::CheckDependenciesOf(SelectedAssets);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("CleanProject_ContentBrowserWhitelistTitle", "Whitelist selected assets"),
		LOCTEXT("CleanProject_ContentBrowserWhitelistTooltip", "Add the selected assets to the whitelist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				auto Settings = GetMutableDefault<UCPProjectSettings>();
				Settings->WhitelistAssetes(SelectedAssets);
			})
		));

	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE

