// Copyright Out-of-the-Box Plugins 2018-2020. All Rights Reserved.

#include "CleanProjectModule.h"

#include "CPMenuExtensions.h"
#include "CPLog.h"


// Unsorted includes.
#include "AssetData.h"
#include "AssetRegistryModule.h"
#include "AssetRegistryModule.h"
#include "AssetTools/Private/SPackageReportDialog.h"
#include "CPEditorSettings.h"
#include "CPProjectSettings.h"
#include "CPOperations.h"
#include "Containers/Array.h"
#include "ContentBrowserDelegates.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserModule.h"
#include "CoreMinimal.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "EditorStyleSet.h"
#include "EditorStyleSet.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "GameFramework/HUD.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "LevelEditor.h"
#include "Misc/FeedbackContext.h"
#include "Misc/MessageDialog.h"
#include "Misc/MessageDialog.h"
#include "Misc/OutputDeviceConsole.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "UnrealEd/Public/ObjectTools.h"

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
#undef LOCTEXT_NAMESPACE