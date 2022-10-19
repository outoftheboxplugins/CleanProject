// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CleanProjectModule.h"

#include "CPLog.h"
#include "CPOperationsSubsystem.h"
#include "CPSettings.h"
#include "Shared/OutOfTheBoxHelpers.h"
#include "Widgets/SCPDashboardWidget.h"

#include <ContentBrowserModule.h>
#include <ToolMenus.h>

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
const FName MenuTabName = FName("CleanProjectMenuTab");
}

void FCleanProjectModule::StartupModule()
{
	LOG_TRACE();

	RegisterContentBrowserExtensions();
	RegisterWindowExtensions();
	RegisterToolsExtensions();
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

	UnregisterContentBrowserExtensions();
	UnregisterWindowExtensions();
	UnregisterToolsExtensions();
}

void FCleanProjectModule::RegisterContentBrowserExtensions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuDelegates =
			ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
		CBAssetMenuDelegates.Add(
			FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCleanProjectModule::CreateCBAssetsExtender));
		CBAssetsExtenderDelegateHandle = CBAssetMenuDelegates.Last().GetHandle();

		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuDelegates =
			ContentBrowserModule->GetAllPathViewContextMenuExtenders();
		CBFolderMenuDelegates.Add(
			FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FCleanProjectModule::CreateCBFoldersExtender));
		CBFoldersExtenderDelegateHandle = CBFolderMenuDelegates.Last().GetHandle();
	}
	else
	{
		UE_LOG(LogCleanProject, Warning, TEXT("ContentBrowser module is not available"));
	}
}

void FCleanProjectModule::UnregisterContentBrowserExtensions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		if (CBAssetsExtenderDelegateHandle.IsValid())
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuDelegates =
				ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			CBAssetMenuDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
				{ return Delegate.GetHandle() == CBAssetsExtenderDelegateHandle; });
		}

		if (CBFoldersExtenderDelegateHandle.IsValid())
		{
			TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuDelegates =
				ContentBrowserModule->GetAllPathViewContextMenuExtenders();
			CBFolderMenuDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedPaths& Delegate)
				{ return Delegate.GetHandle() == CBFoldersExtenderDelegateHandle; });
		}
	}
}

void FCleanProjectModule::RegisterWindowExtensions()
{
	TSharedRef<FWorkspaceItem> const OutOfTheBoxCategory = OutOfTheBoxHelpers::GetSharedWindowsCategory();
	FTabSpawnerEntry& CPMenuTab = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		MenuTabName, FOnSpawnTab::CreateRaw(this, &FCleanProjectModule::CreateDashboardNomadTab));

	CPMenuTab.SetDisplayName(LOCTEXT("DashboardName", "Clean Project Dashboard"))
		.SetTooltipText(LOCTEXT("DashboardTooltip", "Get an overview of your project state in a separate tab."))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "DerivedData.ResourceUsage"))
		.SetGroup(OutOfTheBoxCategory);
}

void FCleanProjectModule::UnregisterWindowExtensions()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MenuTabName);
}

void FCleanProjectModule::RegisterToolsExtensions()
{
	FToolMenuSection& SharedSection = OutOfTheBoxHelpers::GetSharedActionsCategory();
	SharedSection.AddSubMenu("CleanProject", LOCTEXT("CleanProjectCategoryName", "Clean Project"),
		LOCTEXT("CleanProjectCategoryTooltip", "Organise your projet quick and easy by using smart cleanup operations"),
		FNewToolMenuDelegate::CreateRaw(this, &FCleanProjectModule::CreateToolsSubMenu), false,
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Icons.Search"));
}

void FCleanProjectModule::UnregisterToolsExtensions()
{
	UToolMenus::Get()->UnregisterOwner(this);
}

void FCleanProjectModule::CreateToolsSubMenu(UToolMenu* InMenu)
{
	FToolMenuOwnerScoped OwnerScoped(this);
	FToolMenuSection& Section = InMenu->AddSection("Actions");
	Section.AddEntry(FToolMenuEntry::InitMenuEntry("MenuCleanupUnusedAssetsFast",
		LOCTEXT("MenuCleanupUnusedAssetsFast", "Cleanup unused assets Fast"),
		LOCTEXT("MenuCleanupUnusedAssetsFastTooltip", "Uses cached data to determine unused assets in your project."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Fast* from menu."));
				UCPOperationsSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Fast);
			}))));

	Section.AddEntry(FToolMenuEntry::InitMenuEntry("MenuCleanupUnusedAssetsComplex",
		LOCTEXT("MenuCleanupUnusedAssetsComplex", "Cleanup unused assets Complex"),
		LOCTEXT("MenuCleanupUnusedAssetsComplexTooltip",
			"!WARNING: VERY SLOW! Loads all assets to determine unused assets in your project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
					  []()
					  {
						  UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Complex* from menu."));
						  UCPOperationsSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Complex);
					  }),
			FCanExecuteAction::CreateLambda([]() { return false; }))));

	Section.AddEntry(FToolMenuEntry::InitMenuEntry("MenuCleanupRedirects", LOCTEXT("MenuCleanupRedirects", "Cleanup redirects"),
		LOCTEXT("MenuCleanupRedirectsTooltip", "Fix redirects in your whole project."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Redirects* from menu."));
				UCPOperationsSubsystem::Get()->FixUpRedirectsInProject();
			}))));

	Section.AddEntry(
		FToolMenuEntry::InitMenuEntry("MenuCleanupEmptyFolders", LOCTEXT("MenuCleanupEmptyFolders", "Cleanup empty folders"),
			LOCTEXT("MenuCleanupEmptyFoldersTooltip", "Delete all the empty folders from your project."), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(
				[]()
				{
					UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from menu."));
					UCPOperationsSubsystem::Get()->DeleteAllEmptyPackageFolders();
				}))));
}

TSharedRef<SDockTab> FCleanProjectModule::CreateDashboardNomadTab(const FSpawnTabArgs& Args)
{
	// clang-format off
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCPDashboardWidget)
		];
	// clang-format on
}

TSharedRef<FExtender> FCleanProjectModule::CreateCBAssetsExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension("AssetContextAdvancedActions", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FCleanProjectModule::CreateCBAssetsEntry, SelectedAssets));

	return ContentBrowserExtender;
}

void FCleanProjectModule::CreateCBAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("ContentBrowserAssetSection", "Clean Project"));

	MenuBuilder.AddMenuEntry(LOCTEXT("AssetsCheckUnusedFast", "Fast Check if selected assets are unused"),
		LOCTEXT("AssetsCheckUnusedFastTooltip", "Uses cached data to determine unused assets in the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedAssets]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Fast* from selected assets."));
				UCPOperationsSubsystem::Get()->DeleteUnusedAssets(SelectedAssets, EScanType::Fast);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("AssetsCheckUnusedComplex", "Complex Check if selected assets are unused"),
		LOCTEXT("AssetsCheckUnusedComplexTooltip",
			"!WARNING: VERY SLOW! Loads all assets to determine unused assets in the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
					  [SelectedAssets]()
					  {
						  UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Complex* from selected assets."));
						  UCPOperationsSubsystem::Get()->DeleteUnusedAssets(SelectedAssets, EScanType::Complex);
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

TSharedRef<FExtender> FCleanProjectModule::CreateCBFoldersExtender(const TArray<FString>& SelectedFolders)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension("PathContextBulkOperations", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FCleanProjectModule::CreateCBFoldersEntry, SelectedFolders));

	return ContentBrowserExtender;
}

void FCleanProjectModule::CreateCBFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("ContentBrowserFolderSection", "Clean Project"));
	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersCheckUnusedFast", "Fast Check assets from the selected folders"),
		LOCTEXT("FoldersCheckUnusedFastTooltip", "Uses cached data to determine unused assets in the selected folders."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Fast* from selected folders."));
				UCPOperationsSubsystem::Get()->DeleteUnusedAssets(SelectedFolders, EScanType::Fast);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersCheckUnusedComplex", "Complex Check assets from the selected folders"),
		LOCTEXT("FoldersCheckUnusedComplexTooltip",
			"!WARNING: VERY SLOW! Loads all assets to determine unused assets in the selected folders."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
					  [SelectedFolders]()
					  {
						  UE_LOG(LogCleanProject, Log, TEXT("Starting *Check Unused Assets Complex* from selected folders."));
						  UCPOperationsSubsystem::Get()->DeleteUnusedAssets(SelectedFolders, EScanType::Complex);
					  }),
			FCanExecuteAction::CreateLambda([]() { return false; })));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersWhitelistAssets", "Whitelist assets from selected folders"),
		LOCTEXT("FoldersWhitelistAssetsTooltip", "Add the assets from the selected folders to the Whitelist."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Whitelist Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders = UCPOperationsSubsystem::Get()->GetAssetsInPaths(SelectedFolders);
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->WhitelistAssets(AssetsInSelectedFolders);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersBlacklistAssets", "Blacklist assets from selected folders"),
		LOCTEXT("FoldersBlacklistAssetsTooltip", "Add the assets from the selected folders to the Blacklist."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Blacklist Assets* from selected folders."));
				const TArray<FAssetData> AssetsInSelectedFolders = UCPOperationsSubsystem::Get()->GetAssetsInPaths(SelectedFolders);
				UCPSettings* Settings = GetMutableDefault<UCPSettings>();
				Settings->BlacklistAssets(AssetsInSelectedFolders);
			})));

	MenuBuilder.AddMenuEntry(LOCTEXT("FoldersCleanupEmptyFolders", "Cleanup empty folders"),
		LOCTEXT("FoldersCleanupEmptyFoldersTooltip", "Delete all the empty folders from the selected folders."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[SelectedFolders]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from selected folders."));
				UCPOperationsSubsystem::Get()->DeleteEmptyPackageFoldersIn(SelectedFolders);
			})));

	MenuBuilder.EndSection();
}

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject);

#undef LOCTEXT_NAMESPACE
