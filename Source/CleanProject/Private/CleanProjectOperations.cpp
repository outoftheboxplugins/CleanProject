// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectOperations.h"

#include "AssetRegistryModule.h"
#include "Core/Public/Misc/ScopedSlowTask.h"
#include "Core/Public/Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace CleanProjectOperations
{
	void CheckDependenciesBasedOn(TArray<FAssetData> SelectedAssets)
	{
		CheckDependenciesOf(SelectedAssets, GetAllGameAssets());
	}

	void CheckDependenciesOf(TArray<FAssetData> SelectedAssets)
	{
		CheckDependenciesOf(GetAllGameAssets(), SelectedAssets);
	}

	void CheckDependenciesOf(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest)
	{
		// Recursively get all the packages to check from dependencies.
		TSet<FName> AllPackageNamesToCheck;
		
		// Create a slow task to display a progressbar for the user.
		FScopedSlowTask SlowTask(AssetsToTest.Num(), LOCTEXT("CleanProject_SlowTaskTitle", "Gathering Dependencies..."));
		bool showCancelButton = true;
		bool allowPIE = true;
		SlowTask.MakeDialog(showCancelButton, allowPIE);

		for (auto PackageIt = AssetsToTest.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			SlowTask.EnterProgressFrame();

			if (!AllPackageNamesToCheck.Contains(PackageIt->PackageName))
			{
				AllPackageNamesToCheck.Add(PackageIt->PackageName);
				RecursiveGetDependencies(PackageIt->PackageName, AllPackageNamesToCheck);
			}
		}
		

		// Removed the dependenices found from the ones tested.
		for (auto DependsIt = AllPackageNamesToCheck.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			for (auto AssetIt = DependenciesToTest.CreateConstIterator(); AssetIt; ++AssetIt)
			{
				FAssetData current = *AssetIt;
				if (current.PackageName == *DependsIt)
				{
					DependenciesToTest.Remove(current);
					--AssetIt;
				}
			}
		}

		// Confirm that there is at least one package to 
		if (DependenciesToTest.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CleanProject_NoFilesToDelete", "No unused assets found."));
		}
		else
		{










			
			
			
			

			// Prompt the user displaying all assets that are going to be deleted.
			//const FText ReportMessage = LOCTEXT("DepenCheckerReportTitle", "The following assets are not used by the selected assets.");
			//TArray<FString> ReportPackageNames;
			//for (auto PackageIt = DependenciesToTest.CreateConstIterator(); PackageIt; ++PackageIt)
			//{
			//	ReportPackageNames.Add((*PackageIt).PackageName.ToString());
			//}

			//SDependReportDialog::FOnReportConfirmed OnReportConfirmed = SPackageReportDialog::FOnReportConfirmed::CreateRaw(this, &FCleanProjectModule::CheckDepencies_ReportConfirmed, DependenciesToTest);
			//SDependReportDialog::FOnReportConfirmed OnReporBlackListed = SPackageReportDialog::FOnReportConfirmed::CreateRaw(this, &FCleanProjectModule::CheckDepencies_ReportBlackListed, DependenciesToTest);
			//SDependReportDialog::OpenDependReportDialog(ReportMessage, ReportPackageNames, OnReportConfirmed, OnReporBlackListed);
			//
			//
			//if (FModuleManager::Get().ModuleExists(TEXT("AssetManagerEditor")))
			//{
			//	IAssetManagerEditorModule& Module = FModuleManager::LoadModuleChecked< IAssetManagerEditorModule >("AssetManagerEditor");
			//	Module.OpenAssetAuditUI(DependenciesToTest);
			//
			//}
			//else {
			//	UE_LOG(LogTemp, Error, TEXT("AssetManagerEditor plugin is not enabled"));
			//}
		}
	}

	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FName> Dependencies;

		AssetRegistryModule.Get().GetDependencies(PackageName, Dependencies);

		for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			if (!AllDependencies.Contains(*DependsIt))
			{
				//TODO: Make the skippable folders configurable.
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

	TArray<FAssetData> GetAllGameAssets()
	{
		TArray<FAssetData> AllAssetData;

		FARFilter Filter;
		Filter.PackagePaths.Add(TEXT("/Game"));
		Filter.bRecursivePaths = true;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

		return AllAssetData;
	}
}



#undef LOCTEXT_NAMESPACE