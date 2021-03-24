// Copyright Out-of-the-Box Plugins 2018-2020. All Rights Reserved.

#include "CleanProjectModule.h"

#include "CPMenuExtensions.h"
#include "CPLog.h"


// Unsorted includes.
#include "AssetData.h"
#include "AssetRegistryModule.h"
#include "AssetRegistryModule.h"
#include "AssetTools/Private/SPackageReportDialog.h"
#include "CleanProjectGameSettings.h"
#include "CPOperations.h"
#include "CleanProjectSettings.h"
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

/*
namespace
{
	const FName SettingsContainer	= FName("Editor");
	const FName SettingsCategory	= FName("Plugins");
	const FName SettingsSection		= FName("Clean Project");
	
	const FName SettingsGameContainer	= FName("Project");
	const FName SettingsGameCategory	= FName("Plugins");
	const FName SettingsGameSection		= FName("Clean Project");

	const FName MainMenuExtensionHook = FName("FileLoadSave");
}
*/

void FCleanProjectModule::StartupModule()
{
	LOG_TRACE();

	RegisterMenus();




	/*
	// Register content browser right-click
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCleanProjectModule::CreateContentBrowserExtender));
	FDelegateHandle ContentBrowserAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();
	
	// Register the settings entry.
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(SettingsContainer, SettingsCategory, SettingsSection,
			LOCTEXT("CleanProjectSettings", "Clean Project"),
			LOCTEXT("CleanProjectSettingsDescription", "Cleanup and project management improvements."),
			GetMutableDefault<UCleanProjectSettings>());

		SettingsModule->RegisterSettings(SettingsGameContainer, SettingsGameCategory, SettingsGameSection,
			LOCTEXT("CleanProjectSettings", "Clean Project"),
			LOCTEXT("CleanProjectSettingsDescription", "Cleanup and project management improvements."),
			GetMutableDefault<UCleanProjectGameSettings>());
	}
	*/
}

void FCleanProjectModule::ShutdownModule()
{
	LOG_TRACE();

	UnregisterMenus();

	/*
	// Unregister the settings entry.
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings(SettingsContainer, SettingsCategory, SettingsSection);
		SettingsModule->UnregisterSettings(SettingsGameContainer, SettingsGameCategory, SettingsGameSection);
	}

    FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
    if (LevelEditorModule)
    {
        LevelEditorModule->GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
    }
    MenuExtender = nullptr;
	*/
}

void FCleanProjectModule::RegisterMenus()
{
	MenuExtender = CPMenuExtensions::CreateMenuExtender();

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FCleanProjectModule::UnregisterMenus()
{

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