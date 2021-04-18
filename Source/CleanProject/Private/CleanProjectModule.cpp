// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#include "CleanProjectModule.h"

#include "CPEditorSettings.h"
#include "CPLog.h"
#include "CPMenuExtensions.h"
#include "CPProjectSettings.h"

#include "ContentBrowserModule.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"

namespace
{
	const FName SettingsEditorContainer		= FName("Editor");
	const FName SettingsProjectContainer	= FName("Project");	
	const FName SettingsCategory			= FName("Plugins");
	const FName SettingsSection				= FName("Clean Project");
}

void FCleanProjectModule::StartupModule()
{
	LOG_TRACE();

	RegisterMenus();
	RegisterSettings();
	RegisterAssetActions();
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

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
			NSLOCTEXT("CleanProject", "SettingsName", "Clean Project"),
			NSLOCTEXT("CleanProject", "ettingsDescription", "Cleanup and project management improvements."),
			GetMutableDefault<UCPEditorSettings>());

		SettingsModule->RegisterSettings(SettingsProjectContainer, SettingsCategory, SettingsSection,
			NSLOCTEXT("CleanProject", "SettingsName", "Clean Project"),
			NSLOCTEXT("CleanProject", "ettingsDescription", "Cleanup and project management improvements."),
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
		CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(CPMenuExtensions::CreateContentBrowserExtender));
		CBExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();
	}
}

void FCleanProjectModule::UnregisterAssetActions()
{
	if (FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser")))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == CBExtenderDelegateHandle; });
	}
}