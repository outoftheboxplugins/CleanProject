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

	int64 GetAssetDiskSize(const FAssetData& Asset);
	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList);

	int64 GetUnusuedAssetsDiskSize();
	int64 GetUnusuedAssetsDiskSize(TArray<FAssetData> AssetsToTest);

	struct FChildDepedency
	{
		FChildDepedency(const FName& InAssetName);

		bool operator != (const FChildDepedency& Other);

		void AddDependency(const FName& ChildDependency);
		bool HasChildren() const { return ChildDependencies.Num() != 0; }

		FName AssetName;
		TArray<FChildDepedency> ChildDependencies;
	};

	struct FTreeAssetDepedency
	{
		operator TArray<FName>() const { return GetDependenciesAsList(); }
		FChildDepedency& operator [](const FName& OwnerName) { return GetDependencyRecursive(OwnerName, TopLevelDependencies); }

		void AddTopLevelDependency(const FName& AssetName);
		void AddDepedency(const FName& OwnerName, const FName& DependencyName);

		TArray<FName> GetDependenciesAsList() const;

		FChildDepedency& GetDependencyRecursive(const FName& OwnerName, TArray<FChildDepedency>& ChildrenToCheck);
		void GatherDependencyRecursive(TArray<FName>& OutResult, const TArray<FChildDepedency>& ChildrenToCheck) const;

		TArray<FChildDepedency> TopLevelDependencies;
	};

	FTreeAssetDepedency GetAssetDependenciesTree(const TArray<TSharedPtr<FName>>& AssetsNameList);
	FTreeAssetDepedency GetAssetDependenciesTree(const TArray<FName>& AssetsNameList);

	// Recursively get all the dependencies of a certain package.
	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, FTreeAssetDepedency& ResultTreeDependency);

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

