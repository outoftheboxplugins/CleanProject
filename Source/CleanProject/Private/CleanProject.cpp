// Copyright Alexandru pasotee Oprea 2018. All Rights Reserved.

#include "CleanProject.h"

#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "ContentBrowserModule.h"
#include "MultiBoxExtender.h"
#include "LevelEditor.h"
#include "AssetData.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "ScopedSlowTask.h"
#include "AssetRegistryModule.h"
#include "Private/SPackageReportDialog.h"
#include "MessageDialog.h"
#include "SDependReportDialog.h"

#define LOCTEXT_NAMESPACE "FCleanProjectModule"

void FCleanProjectModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register content browser hook
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCleanProjectModule::OnExtendContentBrowserAssetSelectionMenu));
	ContentBrowserAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();

	// add the File->DataValidation menu subsection

	// make an extension to add the DataValidation function menu
	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("FileLoadAndSave", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateStatic(&DataValidationMenuCreationDelegate));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FCleanProjectModule::ShutdownModule()
{

}

// Extend content browser menu for groups of selected assets
TSharedRef<FExtender> FCleanProjectModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FCleanProjectModule::CreateDataValidationContentBrowserAssetMenu, SelectedAssets));

	return Extender;
}

// Extend content browser menu for groups of selected assets
void FCleanProjectModule::CreateDataValidationContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ValidateAssetsTabTitle", "Check Dependencies of these assets"),
		LOCTEXT("ValidateAssetsTooltipText", "Runs data validation on these assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCleanProjectModule::ValidateAssets, SelectedAssets, false))
	);
	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ValidateAssetsAndDependenciesTabTitle", "Check unused assets of these assets"),
		LOCTEXT("ValidateAssetsAndDependenciesTooltipText", "Runs data validation on these assets and all assets they depend on."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCleanProjectModule::ValidateAssets, SelectedAssets, true))
	);
}

void FCleanProjectModule::ValidateAssets(TArray<FAssetData> SelectedAssets, bool bValidateDependencies)
{
	TArray<FName> PackageNames;
	PackageNames.Reserve(SelectedAssets.Num());
	for (int32 AssetIdx = 0; AssetIdx < SelectedAssets.Num(); ++AssetIdx)
	{
		PackageNames.Add(SelectedAssets[AssetIdx].PackageName);
	}

	TSet<FName> AllPackageNamesToMove;
	{
		FScopedSlowTask SlowTask(PackageNames.Num(), LOCTEXT("MigratePackages_GatheringDependencies", "Gathering Dependencies..."));
		SlowTask.MakeDialog();

		for (auto PackageIt = PackageNames.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			SlowTask.EnterProgressFrame();

			if (!AllPackageNamesToMove.Contains(*PackageIt))
			{
				AllPackageNamesToMove.Add(*PackageIt);
				RecursiveGetDependencies(*PackageIt, AllPackageNamesToMove);
			}
		}
	}

	// Confirm that there is at least one package to move 
	if (AllPackageNamesToMove.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MigratePackages_NoFilesToMove", "No files were found to move"));
		return;
	}

	// Prompt the user displaying all assets that are going to be migrated
	{
		const FText ReportMessage = LOCTEXT("MigratePackagesReportTitle", "The following assets will be migrated to another content folder.");
		TArray<FString> ReportPackageNames;
		for (auto PackageIt = AllPackageNamesToMove.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			ReportPackageNames.Add((*PackageIt).ToString());
		}
		SDependReportDialog::FOnReportConfirmed OnReportConfirmed = SPackageReportDialog::FOnReportConfirmed::CreateRaw(this, &FCleanProjectModule::CheckDepencies_ReportConfirmed, ReportPackageNames);
		SDependReportDialog::OpenDependReportDialog(ReportMessage, ReportPackageNames, OnReportConfirmed);
	}

	//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	//
	//TSet<FAssetData> DependentAssets;
	//if (bValidateDependencies)
	//{
	//	for (const FAssetData& Asset : SelectedAssets)
	//	{
	//		FindAssetDependencies(AssetRegistryModule, Asset, DependentAssets);
	//	}
	//
	//	SelectedAssets = DependentAssets.Array();
	//}
	//
	//UDataValidationManager* DataValidationManager = UDataValidationManager::Get();
	//if (DataValidationManager)
	//{
	//	DataValidationManager->ValidateAssets(SelectedAssets, false);
	//}
	
	
}

void FCleanProjectModule::DataValidationMenuCreationDelegate(FMenuBuilder& MenuBuilder)
{
	//MenuBuilder.BeginSection("DataValidation", LOCTEXT("DataValidation", "DataValidation"));
	//MenuBuilder.AddMenuEntry(
	//	TAttribute<FText>::Create(LOCTEXT("DataValidation", "DataValidation")),
	//	LOCTEXT("ValidateDataTooltip", "Validates all user data in content directory."),
	//	FSlateIcon(FEditorStyle::GetStyleSetName(), "DeveloperTools.MenuIcon"),
	//	FUIAction(FExecuteAction::CreateStatic(&FDataValidationModule::Menu_ValidateData)));
	//MenuBuilder.EndSection();
}

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

void FCleanProjectModule::CheckDepencies_ReportConfirmed(TArray<FString> ConfirmedPackageNamesToMigrate) const
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject)