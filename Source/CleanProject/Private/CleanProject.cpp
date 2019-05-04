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
#include "Private/SDependReportDialog.h"
#include "AssetTools/Private/SPackageReportDialog.h"

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
		FUIAction(FExecuteAction::CreateRaw(this, &FCleanProjectModule::DepenChecker, SelectedAssets))
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
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;
	Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());

	AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

	DepenChecker(AllAssetData);
}

// Calculate the unused assets from a list of selected assets
void FCleanProjectModule::DepenChecker(TArray<FAssetData> SelectedAssets)
{
	TArray<FName> PackageNames;
	PackageNames.Reserve(SelectedAssets.Num());
	for (int32 AssetIdx = 0; AssetIdx < SelectedAssets.Num(); ++AssetIdx)
	{
		PackageNames.Add(SelectedAssets[AssetIdx].PackageName);
	}

	TSet<FName> AllPackageNamesToCheck;
	{
		FScopedSlowTask SlowTask(PackageNames.Num(), LOCTEXT("DepenChecker_GatheringDependencies", "Gathering Dependencies..."));
		SlowTask.MakeDialog();

		for (auto PackageIt = PackageNames.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			SlowTask.EnterProgressFrame();

			if (!AllPackageNamesToCheck.Contains(*PackageIt))
			{
				AllPackageNamesToCheck.Add(*PackageIt);
				RecursiveGetDependencies(*PackageIt, AllPackageNamesToCheck);
			}
		}
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

 	AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);
	
	for (auto DependsIt = AllPackageNamesToCheck.CreateConstIterator(); DependsIt; ++DependsIt)
	{
		for (auto AssetIt = AllAssetData.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			FAssetData current = *AssetIt;
			if (current.PackageName == *DependsIt)
			{
				AllAssetData.Remove(current);
				--AssetIt;
			}
		}
	}

	// Confirm that there is at least one package to 
	if (AllAssetData.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("DepenChecker_NoFilesToDelete", "No files to delete."));
		return;
	}

	// Prompt the user displaying all assets that are going to be deleted.
	{
		const FText ReportMessage = LOCTEXT("DepenCheckerReportTitle", "The following assets are not used by the selected assets.");
		TArray<FString> ReportPackageNames;
		for (auto PackageIt = AllAssetData.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			ReportPackageNames.Add((*PackageIt).PackageName.ToString());
		}
		
		SDependReportDialog::FOnReportConfirmed OnReportConfirmed = SPackageReportDialog::FOnReportConfirmed::CreateRaw(this, &FCleanProjectModule::CheckDepencies_ReportConfirmed, AllAssetData);
		SDependReportDialog::OpenDependReportDialog(ReportMessage, ReportPackageNames, OnReportConfirmed);
	}
}

// Get Dependencies recursive for assets
void FCleanProjectModule::RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FName> Dependencies;

	AssetRegistryModule.Get().GetDependencies(PackageName, Dependencies);

	for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
	{
		if (!AllDependencies.Contains(*DependsIt))
		{
			// @todo Make this skip all packages whose root is different than the source package list root. For now we just skip engine content.
			const bool bIsEnginePackage = (*DependsIt).ToString().StartsWith(TEXT("/Engine"));
			const bool bIsScriptPackage = (*DependsIt).ToString().StartsWith(TEXT("/Script"));
			if (!bIsEnginePackage && !bIsScriptPackage)
			{
				AllDependencies.Add(*DependsIt);
				RecursiveGetDependencies(*DependsIt, AllDependencies);
			}
		}
	}
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

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject)