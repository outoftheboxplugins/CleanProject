// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CleanProjectModule.h"

#include "CPLog.h"
#include "CPOperations.h"
#include "Widgets/SCPMenuWidget.h"

#include "ContentBrowserModule.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"


#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const FName MenuTabName					= FName("CleanProjectMenuTab");
}

void FCleanProjectModule::StartupModule()
{
	LOG_TRACE();

	RegisterMenus();
	RegisterAssetActions();
	RegisterMenuSpawner();
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

	UnregisterMenuSpawner();
	UnregisterAssetActions();
}

void FCleanProjectModule::RegisterMenus()
{	
    FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

	FToolMenuSection& Section = Menu->FindOrAddSection("CleanProject");
	Section.AddEntry(FToolMenuEntry::InitMenuEntry(
	"MenuCleanupUnusedAssets",
		LOCTEXT("MenuCleanupUnusedAssets", "Cleanup unused assets"),
		LOCTEXT("MenuCleanupUnusedAssetsTooltip", "Check for unused assets in your project based on your settings."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets* from menu."));
				CPOperations::CheckAllDependencies();
			}),
			FCanExecuteAction()
		)));

	Section.AddEntry(FToolMenuEntry::InitMenuEntry(
	"MenuCleanupRedirects",
		LOCTEXT("MenuCleanupRedirects", "Cleanup redirects"),
		LOCTEXT("MenuCleanupRedirectsTooltip", "Fix redirects in your whole project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Redirects* from menu."));
				CPOperations::FixUpRedirectsInProject();
			}),
			FCanExecuteAction()
		)));

	Section.AddEntry(FToolMenuEntry::InitMenuEntry(
	"MenuCleanupEmptyFolders",
		LOCTEXT("MenuCleanupEmptyFolders", "Cleanup empty folders"),
		LOCTEXT("MenuCleanupEmptyFoldersTooltip", "Delete all the empty folders from your project."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from menu."));
				CPOperations::DeleteEmptyProjectFolders();
			}),
			FCanExecuteAction()
		)));
}

void FCleanProjectModule::RegisterAssetActions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(CPMenuExtensions::CreateContentBrowserAssetsExtender));
		CBAssetsExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();

		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuExtenderDelegates = ContentBrowserModule->GetAllPathViewContextMenuExtenders();
		CBFolderMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedPaths::CreateStatic(CPMenuExtensions::CreateContentBrowserFoldersExtender));
		CBFoldersExtenderDelegateHandle = CBFolderMenuExtenderDelegates.Last().GetHandle();
	}
}

void FCleanProjectModule::UnregisterAssetActions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == CBAssetsExtenderDelegateHandle; });
		
		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuExtenderDelegates = ContentBrowserModule->GetAllPathViewContextMenuExtenders();
		CBFolderMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedPaths& Delegate) { return Delegate.GetHandle() == CBFoldersExtenderDelegateHandle; });
	}
}

void FCleanProjectModule::RegisterMenuSpawner()
{
	FTabSpawnerEntry& CPMenuTab = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MenuTabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SCPMenuWidget)
			];
	}));

	CPMenuTab
		.SetDisplayName(LOCTEXT("MenuTabDisplayName", "Clean Project Menu"))
		.SetTooltipText(LOCTEXT("MenuTabTooltip", "Organize your project and visualize the data behind the process."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FCleanProjectModule::UnregisterMenuSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MenuTabName);
}

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject);

#undef LOCTEXT_NAMESPACE
