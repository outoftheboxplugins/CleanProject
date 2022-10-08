// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CleanProjectModule.h"

#include "CPDependencyWalkerSubsystem.h"
#include "CPLog.h"
#include "CPMenuExtensions.h"
#include "CPOperations.h"
#include "ContentBrowserModule.h"
#include "LevelEditor.h"
#include "Shared/OutOfTheBoxHelpers.h"
#include "ToolMenus.h"
#include "Widgets/SCPMenuWidget.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
const FName MenuTabName = FName("CleanProjectMenuTab");
}

void FCleanProjectModule::StartupModule()
{
	LOG_TRACE();

	RegisterAssetActions();
	RegisterToolWindows();
	RegisterToolActions();
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

	UnregisterAssetActions();
	UnregisterToolWindows();
	UnregisterToolActions();
}

void FCleanProjectModule::RegisterAssetActions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates =
			ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.Add(
			FContentBrowserMenuExtender_SelectedAssets::CreateStatic(CPMenuExtensions::CreateContentBrowserAssetsExtender));
		CBAssetsExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();

		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuExtenderDelegates =
			ContentBrowserModule->GetAllPathViewContextMenuExtenders();
		CBFolderMenuExtenderDelegates.Add(
			FContentBrowserMenuExtender_SelectedPaths::CreateStatic(CPMenuExtensions::CreateContentBrowserFoldersExtender));
		CBFoldersExtenderDelegateHandle = CBFolderMenuExtenderDelegates.Last().GetHandle();
	}
}

void FCleanProjectModule::RegisterToolWindows()
{
	TSharedRef<FWorkspaceItem> const CleanProjectCategory = OutOfTheBoxHelpers::GetSharedWindowsCategory();
	FTabSpawnerEntry& CPMenuTab = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MenuTabName,
		FOnSpawnTab::CreateLambda(
			[](const FSpawnTabArgs& Args) { return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SCPMenuWidget)]; }));

	CPMenuTab.SetDisplayName(LOCTEXT("MenuTabDisplayName", "Clean Project Dashboard"))
		.SetTooltipText(LOCTEXT("MenuTabTooltip", "Organize your project and visualize the data behind the process."))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "DerivedData.ResourceUsage"))
		.SetGroup(CleanProjectCategory);
}

void FCleanProjectModule::RegisterToolActions()
{
	FToolMenuSection& SharedSection = OutOfTheBoxHelpers::GetSharedActionsCategory();
	SharedSection.AddSubMenu("CleanProject", LOCTEXT("MenuActionDisplayName", "Clean Project"), {},
		FNewToolMenuDelegate::CreateRaw(this, &FCleanProjectModule::CreateToolActionEntries), false,
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Icons.Search"));
}

void FCleanProjectModule::CreateToolActionEntries(UToolMenu* InMenu)
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
				UCPDependencyWalkerSubsystem::Get()->CheckAllDependencies(EScanType::Fast);
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
						  UCPDependencyWalkerSubsystem::Get()->CheckAllDependencies(EScanType::Complex);
					  }),
			FCanExecuteAction::CreateLambda([]() { return false; }))));

	Section.AddEntry(FToolMenuEntry::InitMenuEntry("MenuCleanupRedirects", LOCTEXT("MenuCleanupRedirects", "Cleanup redirects"),
		LOCTEXT("MenuCleanupRedirectsTooltip", "Fix redirects in your whole project."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(
			[]()
			{
				UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Redirects* from menu."));
				CPOperations::FixUpRedirectsInProject();
			}))));

	Section.AddEntry(
		FToolMenuEntry::InitMenuEntry("MenuCleanupEmptyFolders", LOCTEXT("MenuCleanupEmptyFolders", "Cleanup empty folders"),
			LOCTEXT("MenuCleanupEmptyFoldersTooltip", "Delete all the empty folders from your project."), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(
				[]()
				{
					UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Empty Folders* from menu."));
					CPOperations::DeleteEmptyProjectFolders();
				}))));
}

void FCleanProjectModule::UnregisterAssetActions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates =
			ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
			{ return Delegate.GetHandle() == CBAssetsExtenderDelegateHandle; });

		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuExtenderDelegates =
			ContentBrowserModule->GetAllPathViewContextMenuExtenders();
		CBFolderMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedPaths& Delegate)
			{ return Delegate.GetHandle() == CBFoldersExtenderDelegateHandle; });
	}
}

void FCleanProjectModule::UnregisterToolWindows()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MenuTabName);
}

void FCleanProjectModule::UnregisterToolActions()
{
	UToolMenus::Get()->UnregisterOwner(this);
}

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject);

#undef LOCTEXT_NAMESPACE
