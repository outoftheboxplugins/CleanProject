// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

namespace CPOperations
{
	/*
	* Shows all all the selected assets which are not used by any other asset in the content browser.
	* Assets to test: selected assets
	* Dependencies of: all game assets
	*/
	void CheckAllDependencies();
	void CheckDependenciesOf(TArray<FAssetData> SelectedAssets);

	// Return which of the assets are unusued based on the selected one.
	TArray<FAssetData> CheckForUnusuedAssets();
	TArray<FAssetData> CheckForUnusuedAssets(TArray<FAssetData> AssetsToTest);
	TArray<FAssetData> CheckForUnusuedAssets(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest);

	int64 GetAssetDiskSize(const FAssetData& Asset);
	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList);

	int64 GetUnusuedAssetsDiskSize();
	int64 GetUnusuedAssetsDiskSize(TArray<FAssetData> AssetsToTest);

	// Recursively get all the dependencies of a certain package.
	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies);

	// Generate the blacklist for a specific platform or configuration.
	void GenerateBlacklist(const TArray<FAssetData>& AssetsToBlacklist, const bool bAppend, const FString& Platform = "", const FString& Configuration = "");

	// Fix up the redirectors in the whole project.
	void FixUpRedirectorsInProject();

	// Delete all the empty folders of the project.
	void DeleteEmptyProjectFolders();

	// Returns all the assets from the project (Game folder).
	template<typename T>
	TArray<FAssetData> GetAllGameAssets();
	TArray<FAssetData> GetAllGameAssets(TArray<FName> ClassTypes = TArray<FName>());
};

template<typename T>
TArray<FAssetData> CPOperations::GetAllGameAssets()
{
	TArray<FName> MapsClassFilter;
	MapsClassFilter.Add(T::StaticClass()->GetFName());

	return CPOperations::GetAllGameAssets(MapsClassFilter);
}

