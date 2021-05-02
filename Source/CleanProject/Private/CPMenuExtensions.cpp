// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#include "CPMenuExtensions.h"

#include "CPMenuWidget.h"
#include "CPOperations.h"

#include "Framework/MultiBox/MultiBoxExtender.h" // for FExtender
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const FName MainMenuExtensionHook = FName("FileLoadAndSave");
}

namespace Helpers
{
	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths)
	{
		FARFilter Filter;
		Filter.bRecursivePaths = true;

		for (const FString& FolderPath : FolderPaths)
		{
			Filter.PackagePaths.Add(FName(FolderPath));
		}

		TArray<FAssetData> AllAssetData;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

		return AllAssetData;
	}
}

TSharedRef<SDockTab> CPMenuExtensions::SpawnMenuTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCPMenuWidget)
		];
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
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets* from menu."));

				CPOperations::CheckAllDependencies();
			})

		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanupRedirects", "Cleanup redirects"),
		LOCTEXT("CleanupRedirectsTooltip", "Fix redirects in your whole project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Redirects* from menu."));

				CPOperations::FixUpRedirectorsInProject();
			})
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanupEmptyFolders", "Cleanup empty folders"),
		LOCTEXT("CleanupEmptyFoldersTooltip", "Delete all the empty folders from your project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from menu."));

				CPOperations::DeleteEmptyProjectFolders();
			})
		));

	MenuBuilder.EndSection();
}

TSharedRef<FExtender> CPMenuExtensions::CreateContentBrowserAssetsExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(CPMenuExtensions::CreateContentBrowserAssetsEntry, SelectedAssets));

	return ContentBrowserExtender;
}

void CPMenuExtensions::CreateContentBrowserAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("ContentBrowserAssetSection", "Clean Project"));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ContentBrowserAssetsCheckUnused", "Check if selected assets are unused"),
		LOCTEXT("ContentBrowserAssetsCheckUnusedTooltip", "Checks if the selected assets are not used by any of the Levels or whitelisted assets"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets* from selected assets."));

				CPOperations::CheckDependenciesOf(SelectedAssets);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ContentBrowserAssetsWhitelistAssets", "Whitelist selected assets"),
		LOCTEXT("ContentBrowserAssetsWhitelistAssetsTooltip", "Add the selected assets to the Whitelist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Whitelist Assets* from selected assets."));

				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->WhitelistAssets(SelectedAssets);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ContentBrowserAssetsBlacklistAssets", "Blacklist selected assets"),
		LOCTEXT("ContentBrowserAssetsBlacklistAssetsTooltip", "Add the selected assets to the Blacklist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected assets."));

				__debugbreak();
			})
		));

	MenuBuilder.EndSection();
}

TSharedRef<FExtender> CPMenuExtensions::CreateContentBrowserFoldersExtender(const TArray<FString>& SelectedFolders)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension(
		"PathContextBulkOperations",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateStatic(CPMenuExtensions::CreateContentBrowserFoldersEntry, SelectedFolders));

	return ContentBrowserExtender;
}

void CPMenuExtensions::CreateContentBrowserFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("ContentBrowserFolderSection", "Clean Project"));
	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ContentBrowserFoldersCheckUnused", "Check if assets from the selected folders are unused"),
		LOCTEXT("ContentBrowserFoldersCheckUnusedTooltip", "Checks if the assets from the selected folders are not used by any of the Levels or whitelisted assets"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets* from selected folders."));

				TArray<FAssetData> AssetsInSelectedFolders = Helpers::GetAssetsInPaths(SelectedFolders);
				CPOperations::CheckDependenciesOf(AssetsInSelectedFolders);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ContentBrowserFoldersWhitelistAssets", "Whitelist assets from selected folders"),
		LOCTEXT("ContentBrowserFoldersWhitelistAssetsTooltip", "Add the selected assets to the Whitelist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Whitelist Assets* from selected folders."));

				TArray<FAssetData> AssetsInSelectedFolders = Helpers::GetAssetsInPaths(SelectedFolders);

				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->WhitelistAssets(AssetsInSelectedFolders);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ContentBrowserFoldersBlacklistAssets", "Blacklist selected assets"),
		LOCTEXT("ContentBrowserFoldersBlacklistAssetsTooltip", "Add the selected assets to the Blacklist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected folders."));

				TArray<FAssetData> AssetsInSelectedFolders = Helpers::GetAssetsInPaths(SelectedFolders);

				__debugbreak();
			})
		));

	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE