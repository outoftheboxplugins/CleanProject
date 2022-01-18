// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuExtensions.h"

#include "CPMenuWidget.h"
#include "CPOperations.h"
#include "CPLog.h"
#include "CPSettings.h"

#include "Framework/MultiBox/MultiBoxExtender.h" // for FExtender
#include "AssetRegistryModule.h"
#include "SCPBlacklistDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const FName MainMenuExtensionHook = FName("FileLoadAndSave");
}

TSharedRef<SDockTab> CPMenuExtensions::SpawnMenuTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCPMenuWidget)
		];
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
		LOCTEXT("AssetsCheckUnused", "Check if selected assets are unused"),
		LOCTEXT("AssetsCheckUnusedTooltip", "Checks if the selected assets are not used based on your settings."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets* from selected assets."));
				CPOperations::CheckDependenciesOf(SelectedAssets);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("AssetsWhitelistAssets", "Whitelist selected assets"),
		LOCTEXT("AssetsWhitelistAssetsTooltip", "Add the selected assets to the Whitelist."),
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
		LOCTEXT("AssetsBlacklistAssets", "Blacklist selected assets"),
		LOCTEXT("AssetsBlacklistAssetsTooltip", "Add the selected assets to the Blacklist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected assets."));
				SCPBlacklistDialog::OpenBlacklistDialog(SelectedAssets);
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
		LOCTEXT("FoldersCheckUnused", "Check if assets from the selected folders are unused"),
		LOCTEXT("FoldersCheckUnusedTooltip", "Checks if the assets from the selected folders are not used based on your settings."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders = CPOperations::GetAssetsInPaths(SelectedFolders);
				CPOperations::CheckDependenciesOf(AssetsInSelectedFolders);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("FoldersWhitelistAssets", "Whitelist assets from selected folders"),
		LOCTEXT("FoldersWhitelistAssetsTooltip", "Add the assets from the selected folders to the Whitelist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Whitelist Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders = CPOperations::GetAssetsInPaths(SelectedFolders);
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->WhitelistAssets(AssetsInSelectedFolders);
			})
		));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("FoldersBlacklistAssets", "Blacklist assets from selected folders"),
		LOCTEXT("FoldersBlacklistAssetsTooltip", "Add the assets from the selected folders to the Blacklist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders = CPOperations::GetAssetsInPaths(SelectedFolders);
				SCPBlacklistDialog::OpenBlacklistDialog(AssetsInSelectedFolders);
			})
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("FoldersCleanupEmptyFolders", "Cleanup empty folders"),
		LOCTEXT("FoldersCleanupEmptyFoldersTooltip", "Delete all the empty folders from the selected folders."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from selected folders."));
				CPOperations::DeleteEmptyProjectFolders(SelectedFolders);
			})
		));

	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE
