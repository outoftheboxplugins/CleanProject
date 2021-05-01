// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#include "CleanProjectModule.h"

#include "CPEditorSettings.h"
#include "CPLog.h"
#include "CPMenuExtensions.h"
#include "CPProjectSettings.h"

#include "ContentBrowserModule.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const FName SettingsEditorContainer		= FName("Editor");
	const FName SettingsProjectContainer	= FName("Project");	
	const FName SettingsCategory			= FName("Plugins");
	const FName SettingsSection				= FName("Clean Project");

	const FName MenuTabName					= FName("CleanProjectMenuTab");
}

/* STATIC */ void FCleanProjectModule::OpenCleanProjectSettings()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(SettingsProjectContainer, SettingsCategory, SettingsSection);
}

void FCleanProjectModule::StartupModule()
{
	LOG_TRACE();

	RegisterMenus();
	RegisterSettings();
	RegisterAssetActions();
	RegisterMenuSpawner();
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

	UnregisterMenuSpawner();
	UnregisterAssetActions();
	UnregisterSettings();
	UnregisterMenus();
}

void FCleanProjectModule::RegisterMenus()
{
	MenuExtender = CPMenuExtensions::CreateMenuExtender();

	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
	    LevelEditorModule->GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
}

void FCleanProjectModule::UnregisterMenus()
{
	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		LevelEditorModule->GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
	}

	MenuExtender = nullptr;
}

void FCleanProjectModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(SettingsEditorContainer, SettingsCategory, SettingsSection,
			LOCTEXT("EditorSettingsName", "Clean Project"),
			LOCTEXT("EditorSettingsDescription", "Cleanup and project management improvements within this editor."),
			GetMutableDefault<UCPEditorSettings>());

		SettingsModule->RegisterSettings(SettingsProjectContainer, SettingsCategory, SettingsSection,
			LOCTEXT("ProjectSettingsName", "Clean Project"),
			LOCTEXT("ProjectSettingsDescription", "Cleanup and project management improvements within this project."),
			GetMutableDefault<UCPProjectSettings>());
	}
}

void FCleanProjectModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings(SettingsEditorContainer, SettingsCategory, SettingsSection);
		SettingsModule->UnregisterSettings(SettingsProjectContainer, SettingsCategory, SettingsSection);
	}
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
	FTabSpawnerEntry& WatchTab = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MenuTabName, FOnSpawnTab::CreateStatic(CPMenuExtensions::SpawnMenuTab));

	WatchTab
		.SetDisplayName(LOCTEXT("MenuTabDisplayName", "Clean Project Menu"))
		.SetTooltipText(LOCTEXT("MenuTabTooltip", "Organize your project and visualize the data behind the process."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FCleanProjectModule::UnregisterMenuSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MenuTabName);
}

#undef LOCTEXT_NAMESPACE