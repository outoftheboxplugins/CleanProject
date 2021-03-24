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



	/*
	// Register content browser right-click
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCleanProjectModule::CreateContentBrowserExtender));
	FDelegateHandle ContentBrowserAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();
	
	*/
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

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

/*
TSharedRef<FExtender> FCleanProjectModule::CreateContentBrowserExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> ContentBrowserExtender = MakeShareable(new FExtender);
	ContentBrowserExtender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FCleanProjectModule::CreateContentBrowserEntry, SelectedAssets));

	return ContentBrowserExtender;
}

void FCleanProjectModule::CreateContentBrowserEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
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
				CleanProjectOperations::CheckDependenciesBasedOn(SelectedAssets); 
			})
	));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("CleanProject_ContentBrowserForTitle", "Check if selected assets are unused"),
		LOCTEXT("CleanProject_ContentBrowserForTooltip", "Returns the unused assets from the list of selected assets based on the dependencies of all."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				CleanProjectOperations::CheckDependenciesOf(SelectedAssets);
			})
	));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("CleanProject_ContentBrowserWhitelistTitle", "Whitelist selected assets"),
		LOCTEXT("CleanProject_ContentBrowserWhitelistTooltip", "Add the selected assets to the whitelist."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
			{
				auto Settings = GetMutableDefault<UCleanProjectGameSettings>();
				Settings->WhitelistAssetes(SelectedAssets);
			})
	));

	MenuBuilder.EndSection();
}

*/

#undef LOCTEXT_NAMESPACE