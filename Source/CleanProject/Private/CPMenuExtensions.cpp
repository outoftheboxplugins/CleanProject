// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPMenuExtensions.h"

#include "CPDependencyWalkerSubsystem.h"
#include "CPLog.h"
#include "CPSettings.h"

#define LOCTEXT_NAMESPACE "CleanProject"

TSharedRef<FExtender> CPMenuExtensions::CreateContentBrowserAssetsExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension("AssetContextAdvancedActions", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateStatic(CreateContentBrowserAssetsEntry, SelectedAssets));

	return ContentBrowserExtender;
}

void CPMenuExtensions::CreateContentBrowserAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("ContentBrowserAssetSection", "Clean Project"));

	MenuBuilder.AddMenuEntry(LOCTEXT("AssetsCheckUnusedFast", "Fast Check if selected assets are unused"),
		LOCTEXT("AssetsCheckUnusedFastTooltip", "Uses cached data to determine unused assets in the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Fast* from selected assets."));
				UCPDependencyWalkerSubsystem::Get()->DeleteUnusedAssets(SelectedAssets, EScanType::Fast);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("AssetsCheckUnusedComplex", "Complex Check if selected assets are unused"),
		LOCTEXT("AssetsCheckUnusedComplexTooltip",
			"!WARNING: VERY SLOW! Loads all assets to determine unused assets in the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
					  [SelectedAssets]()
					  {
						  UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Complex* from selected assets."));
						  UCPDependencyWalkerSubsystem::Get()->DeleteUnusedAssets(SelectedAssets, EScanType::Complex);
					  }),
			FCanExecuteAction::CreateLambda([]() { return false; })));

	MenuBuilder.AddMenuEntry(LOCTEXT("AssetsWhitelistAssets", "Whitelist selected assets"),
		LOCTEXT("AssetsWhitelistAssetsTooltip", "Add the selected assets to the Whitelist."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Whitelist Assets* from selected assets."));
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->WhitelistAssets(SelectedAssets);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("AssetsBlacklistAssets", "Blacklist selected assets"),
		LOCTEXT("AssetsBlacklistAssetsTooltip", "Add the selected assets to the Blacklist."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected assets."));
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->BlacklistAssets(SelectedAssets);
			})));

	MenuBuilder.EndSection();
}

TSharedRef<FExtender> CPMenuExtensions::CreateContentBrowserFoldersExtender(const TArray<FString>& SelectedFolders)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension("PathContextBulkOperations", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateStatic(CreateContentBrowserFoldersEntry, SelectedFolders));

	return ContentBrowserExtender;
}

void CPMenuExtensions::CreateContentBrowserFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("ContentBrowserFolderSection", "Clean Project"));
	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersCheckUnusedFast", "Fast Check assets from the selected folders"),
		LOCTEXT("FoldersCheckUnusedFastTooltip", "Uses cached data to determine unused assets in the selected folders."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Fast* from selected folders."));
				UCPDependencyWalkerSubsystem::Get()->DeleteUnusedAssets(SelectedFolders, EScanType::Fast);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersCheckUnusedComplex", "Complex Check assets from the selected folders"),
		LOCTEXT("FoldersCheckUnusedComplexTooltip",
			"!WARNING: VERY SLOW! Loads all assets to determine unused assets in the selected folders."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
					  [SelectedFolders]()
					  {
						  UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Complex* from selected folders."));
						  UCPDependencyWalkerSubsystem::Get()->DeleteUnusedAssets(SelectedFolders, EScanType::Complex);
					  }),
			FCanExecuteAction::CreateLambda([]() { return false; })));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersWhitelistAssets", "Whitelist assets from selected folders"),
		LOCTEXT("FoldersWhitelistAssetsTooltip", "Add the assets from the selected folders to the Whitelist."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Whitelist Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders =
					UCPDependencyWalkerSubsystem::Get()->GetAssetsInPaths(SelectedFolders);
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->WhitelistAssets(AssetsInSelectedFolders);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersBlacklistAssets", "Blacklist assets from selected folders"),
		LOCTEXT("FoldersBlacklistAssetsTooltip", "Add the assets from the selected folders to the Blacklist."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders =
					UCPDependencyWalkerSubsystem::Get()->GetAssetsInPaths(SelectedFolders);
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->BlacklistAssets(AssetsInSelectedFolders);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersCleanupEmptyFolders", "Cleanup empty folders"),
		LOCTEXT("FoldersCleanupEmptyFoldersTooltip", "Delete all the empty folders from the selected folders."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from selected folders."));
				UCPDependencyWalkerSubsystem::Get()->DeleteEmptyPackageFoldersIn(SelectedFolders);
			})));

	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE
