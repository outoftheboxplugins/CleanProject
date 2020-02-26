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
#include "Engine/World.h"
#include "AssetTools/Private/SPackageReportDialog.h"
#include "CleanProjectSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "AssetManagerEditorModule.h"
#include "CleanProject\Public\CleanProjectOperations.h"

#define LOCTEXT_NAMESPACE "FCleanProjectModule"

void FCleanProjectModule::StartupModule()
{
	// Register content browser right-click
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCleanProjectModule::OnExtendContentBrowserAssetActions));
	FDelegateHandle ContentBrowserAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();

	// Register main menu dropdown entry
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("FileLoadAndSave", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FCleanProjectModule::CreateDepenCheckerMainMenuEntry));
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	// Register the settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Editor", "Plugins", "Clean Project",
			LOCTEXT("CleanProjectSettings", "Clean Project Settings"),
			LOCTEXT("CleanProjectSettingsDescription", "Cleanup and project management improvements."),
			GetMutableDefault<UCleanProjectSettings>()
		);
	}
}

void FCleanProjectModule::ShutdownModule()
{
	//TODO: unregister stuff.
}

// Extend content browser menu for to add depend checker delegate
TSharedRef<FExtender> FCleanProjectModule::OnExtendContentBrowserAssetActions(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FCleanProjectModule::CreateDepenCheckerContentBrowserAssetAction, SelectedAssets));

	return Extender;
}

// Create the menu entry for the content browser
void FCleanProjectModule::CreateDepenCheckerContentBrowserAssetAction(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("DepenCheckerTabTitle", "Check unused assets"),
		LOCTEXT("DepenCheckerTooltipText", "Returns all the assets not used by the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([&SelectedAssets]() { CleanProjectOperations::CheckDependenciesBasedOn(SelectedAssets); }))
	);
}

// Extend main menu for to add depend checker delegate
void FCleanProjectModule::CreateDepenCheckerMainMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("CleanProject", LOCTEXT("CleanProject", "CleanProject"));
	MenuBuilder.AddMenuEntry(
		FText(LOCTEXT("DepenChecker", "DepenChecker")),
		LOCTEXT("DepenCheckerTooltip", "Return all the unused assets of the game."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "DeveloperTools.MenuIcon"),
		FUIAction(FExecuteAction::CreateRaw(this, &FCleanProjectModule::OnExtendMainMenu)));
	MenuBuilder.EndSection();
}

// Create the menu entry for the main menu
void FCleanProjectModule::OnExtendMainMenu()
{
	CleanProjectOperations::CheckDependenciesBasedOn(CleanProjectOperations::GetAllGameAssets());
}



// Delete the assets when a report is confirmed
void FCleanProjectModule::CheckDepencies_ReportConfirmed(TArray<FAssetData> ConfirmedPackageNamesToDelete) const
{
	TArray<UObject*> AssetsToDelete;
	for (auto AssetIt = ConfirmedPackageNamesToDelete.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		FAssetData current = *AssetIt;
		AssetsToDelete.Add(current.GetAsset());
	}

	const bool bShowConfirmation = false;
	ObjectTools::ForceDeleteObjects(AssetsToDelete, bShowConfirmation);
}

void FCleanProjectModule::CheckDepencies_ReportBlackListed(TArray<FAssetData> ConfirmedPackageNamesToBlackList) const
{
	FString FileContent;
	for (auto PackageIt = ConfirmedPackageNamesToBlackList.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		FString assetPath = (*PackageIt).PackageName.ToString();
		FileContent += FString::Printf(TEXT("../../..%s\n"), *assetPath);
	}

	auto Settings = GetDefault<UCleanProjectSettings>();
	if (Settings->bUseSmartBlackList)
	{
		FString projectBuildRoot = FPaths::ProjectDir() + "Build";

		for (const FString& platformFolder : Settings->PlatformsPaths)
		{
			for (const FString& listFile : Settings->BlacklistFiles)
			{
				FString slash = FGenericPlatformMisc::GetDefaultPathSeparator();
				FString platformPath = projectBuildRoot + slash + platformFolder + slash + listFile;
				FFileHelper::SaveStringToFile(FileContent, *platformPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
			}
		}
	}
	else
	{
		FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Blacklist.txt");
		
		FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
		FPlatformProcess::LaunchURL(*FString::Printf(TEXT("file://%s"), *FilePath), NULL, NULL);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject)