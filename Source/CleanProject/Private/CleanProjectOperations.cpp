// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectOperations.h"

#include "AssetRegistryModule.h"
#include "Core/Public/Misc/ScopedSlowTask.h"
#include "Core/Public/Misc/MessageDialog.h"
#include "Engine/World.h"
#include "SCleanProjectAssetDialog.h"
#include "CleanProjectSettings.h"
#include "CleanProjectGameSettings.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace CleanProjectOperations
{
	void CheckDependenciesBasedOn(TArray<FAssetData> SelectedAssets)
	{
		CheckDependenciesInternal(GetAllGameAssets(), SelectedAssets);
	}

	void CheckDependenciesOf(TArray<FAssetData> SelectedAssets)
	{
		// Remove the selected from all the game assets before proceding.
		TArray<FAssetData> FilteredGameAssets = GetAllGameAssets();
		FilteredGameAssets.RemoveAllSwap([&SelectedAssets](const FAssetData& current)
			{
				return SelectedAssets.Contains(current);
			});

		CheckDependenciesInternal(SelectedAssets, FilteredGameAssets);
	}

	void CheckDependenciesInternal(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest)
	{
		auto Settings = GetDefault<UCleanProjectGameSettings>();

		for (const FName& WhiteListAssetPath : Settings->WhiteListAssetsPaths)
		{
			AssetsToTest.RemoveAllSwap([&WhiteListAssetPath](const FAssetData& AssetData)
				{
					return AssetData.ObjectPath == WhiteListAssetPath;
				});
		}

		for (auto PackageIt = DependenciesToTest.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			AssetsToTest.RemoveSwap(*PackageIt);
		}

		// Recursively get all the packages to check from dependencies.
		TSet<FName> AllPackageNamesToCheck;
		
		// Create a slow task to display a progressbar for the user.
		FScopedSlowTask SlowTask(DependenciesToTest.Num(), LOCTEXT("CleanProject_SlowTaskTitle", "Gathering Dependencies..."));
		bool showCancelButton = true;
		bool allowPIE = true;
		SlowTask.MakeDialog(showCancelButton, allowPIE);

		for (auto PackageIt = DependenciesToTest.CreateConstIterator(); PackageIt; ++PackageIt)
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
			const FName& DependencyPackageName = *DependsIt;

			AssetsToTest.RemoveAllSwap([&DependencyPackageName](const FAssetData& AssetData)
				{
					return AssetData.PackageName == DependencyPackageName;
				});
		}

		// Confirm that there is at least one package to 
		if (AssetsToTest.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CleanProject_NoFilesToDelete", "No unused assets found."));
		}
		else
		{
			SCleanProjectAssetDialog::OpenAssetDialog(AssetsToTest);
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

	TArray<FAssetData> GetAllGameAssets(TArray<FName> ClassTypes /* = TArray<FName>()*/)
	{
		TArray<FAssetData> AllAssetData;

		FARFilter Filter;
		Filter.PackagePaths.Add(TEXT("/Game"));
		Filter.bRecursivePaths = true;

		Filter.ClassNames.Reserve(ClassTypes.Num());
		for (const auto& classType : ClassTypes)
		{
			Filter.ClassNames.Add(classType);
		}
		
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

		return AllAssetData;
	}

	TArray<FAssetData> GetAllMapAssets()
	{
		TArray<FName> MapsClassFilter;
		MapsClassFilter.Add(UWorld::StaticClass()->GetFName());

		return GetAllGameAssets(MapsClassFilter);
	}
}

#undef LOCTEXT_NAMESPACE