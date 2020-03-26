// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace CleanProjectOperations
{
    /*
    * Shows all other assets from the content browser which are not used by the selected assets or their dependencies.
    * Assets to test: all game assets
    * Dependencies of: selected assets
    */
    void CheckDependenciesBasedOn(TArray<FAssetData> SelectedAssets);

    /*
    * Shows all all the selected assets which are not used by any other asset in the content browser.
    * Assets to test: selected assets
    * Dependencies of: all game assets
    */
    void CheckDependenciesOf(TArray<FAssetData> SelectedAssets);

    // Check if the AssetsToTest are used by any of the DependenciesToTest.
    void CheckDependenciesInternal(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest);

    // Recursively get all the dependencies of a certain package.
    void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies);

	// Generate the blacklist for a specific platform or configuration.
	void GenerateBlacklist(const TArray<FAssetData>& AssetsToBlacklist, const bool bAppend, const FString& Platform = "", const FString& Configuration = "");

    // Fix up the redirectors in the whole project.
    void FixUpRedirectorsInProject();

    // Delete all the empty folders of the project.
    void DeleteEmptyProjectFolders();

    // Returns all the assets from the project (Game folder).
    TArray<FAssetData> GetAllGameAssets(TArray<FName> ClassTypes = TArray<FName>());

    // Returns all the map assets from the project (Game folder).
    TArray<FAssetData> GetAllMapAssets();

	// Helpers

    // Returns the selection from a list, or the whole list if selection was not found.
	TArray<FString> GetListFromSelection(const TArray<FString>& List, const FString& Selection);

    // Loads the AssetData of redirects and populates a list with the loaded objects.
    // Returns true if all the objects were loaded successfully or false if any of them fails.
    bool LoadRedirectAssetsInProject(TArray<UObject*>& Objects);

    void GetEmptyFolderInPath(const FString& BaseDirectory, TArray<FString>& OutEmptyFolders);
}