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
    * Shows all all the selected assets which are not used by any other asset in the content broweser.
    * Assets to test: selected assets
    * Dependencies of: all game assets
    */
    void CheckDependenciesOf(TArray<FAssetData> SelectedAssets);

    // Check if the AssetsToTest are used by any of the DependenciesToTest.
    void CheckDependenciesInternal(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest);

    // Recursively get all the dependencies of a certain package.
    void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies);

    // Returns all the assets from the project (Game folder).
    TArray<FAssetData> GetAllGameAssets(TArray<FName> ClassTypes = TArray<FName>());

    // Returns all the map assets from the project (Game folder).
    TArray<FAssetData> GetAllMapAssets();
}