// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProject.h"

#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "ContentBrowserModule.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Misc/ScopedSlowTask.h"
#include "AssetRegistryModule.h"
#include "Misc/MessageDialog.h"
#include "EngineUtils.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "EditorStyleSet.h"
#include "AssetTools/Private/SPackageReportDialog.h"
#include "Misc/Paths.h"

#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "GameFramework/HUD.h"

#include "Framework/Application/SlateApplication.h"
#include "ToolMenus.h"
#include "EditorStyleSet.h"
#include "AssetRegistryModule.h"
#include "Misc/MessageDialog.h"
#include "LevelEditor.h"
#include "Misc/FeedbackContext.h"
#include "Misc/OutputDeviceConsole.h"
#include "Modules/ModuleManager.h"

#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"

//#include "WorkspaceMenuStructure.h"
//#include "WorkspaceMenuStructureModule.h"
//#include "EditorValidatorSubsystem.h"



#include "CleanProjectSettings.h"
#include "CleanProject\Public\CleanProjectOperations.h"

#define LOCTEXT_NAMESPACE "FCleanProjectModule"

namespace
{
	const FName SettingsContainer = FName("Editor");
	const FName SettingsCategory = FName("Plugins");
	const FName SettingsSection = FName("Clean Project");

	const FName MainMenuExtensionHook = FName("FileLoadSave");
}

void FCleanProjectModule::StartupModule()
{
	// Register content browser right-click
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCleanProjectModule::CreateContentBrowserExtender));
	FDelegateHandle ContentBrowserAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();
	
	// Hook the extender to the editor module.
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FCleanProjectModule::RegisterMenus);

	// Register the settings entry.
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(SettingsContainer, SettingsCategory, SettingsSection,
			LOCTEXT("CleanProjectSettings", "Clean Project Settings"),
			LOCTEXT("CleanProjectSettingsDescription", "Cleanup and project management improvements."),
			GetMutableDefault<UCleanProjectSettings>());
	}
}

void FCleanProjectModule::ShutdownModule()
{
	// Unregister the settings entry.
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings(SettingsContainer, SettingsCategory, SettingsSection);
	}

	// Unregister main menu dropdown entry.
	UToolMenus::UnregisterOwner(this);
}

void FCleanProjectModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.File");
	FToolMenuSection& Section = Menu->AddSection("Plugins",
		LOCTEXT("CleanProjectSection", "Plugins"),
		FToolMenuInsert("FileLoadAndSave", EToolMenuInsertType::After));

	Section.AddEntry(FToolMenuEntry::InitMenuEntry(
		"ValidateData",
		LOCTEXT("CleanProjectMaiMenu", "Clean your project"),
		LOCTEXT("CleanProjectMainMenuTooltip", "Check depedencies based on all your maps."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "DeveloperTools.MenuIcon"),
		FUIAction(FExecuteAction::CreateLambda([]()
			{
				TArray<FAssetData> MapAssetDatas = CleanProjectOperations::GetAllMapAssets();
				CleanProjectOperations::CheckDependenciesBasedOn(MapAssetDatas);
			})
		)));
}

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
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject)