// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "AssetRegistry\AssetData.h"
#include "Engine\Classes\Engine\World.h"

using FAssetDataPtr = TSharedPtr<FName>;

namespace CPOperations
{
	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths);

	/*
	* Shows all all the selected assets which are not used by any other asset in the content browser.
	* Assets to test: selected assets
	* Dependencies of: all game assets
	*/
	void CheckAllDependencies();
	void CheckDependenciesOf(TArray<FAssetData> SelectedAssets);

	// Return which of the assets are unused based on the selected one.
	TArray<FAssetData> CheckForUnusedAssets();
	TArray<FAssetData> CheckForUnusedAssets(TArray<FAssetData> AssetsToTest);

	int64 GetAssetDiskSize(const FAssetData& Asset);
	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList);

	int64 GetUnusedAssetsDiskSize();
	int64 GetUnusedAssetsDiskSize(TArray<FAssetData> AssetsToTest);

	struct FChildDependency
	{
		explicit FChildDependency(const FName& InAssetName);

		bool operator != (const FChildDependency& Other) const;

		void AddDependency(const FName& ChildDependency);
		bool HasChildren() const { return ChildDependencies.Num() != 0; }

		FName GetAssetName() const { return *AssetName; }
		TArray<FAssetDataPtr> GetChildrenAssetPtrs();

		FAssetDataPtr AssetName;
		TArray<FChildDependency> ChildDependencies;
	};

	struct FTreeAssetDependency
	{
		operator TArray<FName>() const { return GetDependenciesAsList(); }
		FChildDependency& operator [](const FName& OwnerName) { return GetDependencyRecursive(OwnerName, TopLevelDependencies); }

		void AddTopLevelDependency(const FName& AssetName);
		void AddDependency(const FName& OwnerName, const FName& DependencyName);

		TArray<FName> GetDependenciesAsList() const;

		FChildDependency& GetDependencyRecursive(const FName& OwnerName, TArray<FChildDependency>& ChildrenToCheck);
		void GatherDependencyRecursive(TArray<FName>& OutResult, const TArray<FChildDependency>& ChildrenToCheck) const;

		TArray<FChildDependency> TopLevelDependencies;
		TArray<FAssetDataPtr> TopLevelAssetsPtr;
	};

	FTreeAssetDependency GetAssetDependenciesTree(const TArray<FAssetDataPtr>& AssetsNameList);
	FTreeAssetDependency GetAssetDependenciesTree(const TArray<FName>& AssetsNameList);

	// Recursively get all the dependencies of a certain package.
	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, FTreeAssetDependency& ResultTreeDependency, TSet<FString>& ExternalActorsPaths);

	// Generate the blacklist for a specific platform or configuration.
	void GenerateBlacklist(const TArray<FAssetData>& AssetsToBlacklist, const bool bAppend, const FString& Platform = "", const FString& Configuration = "");

	// Fix up the redirects in the whole project.
	void FixUpRedirectsInProject();

	// Delete all the empty folders of the project.
	void DeleteEmptyProjectFolders();
	void DeleteEmptyProjectFolders(TArray<FString> SelectedFolders);

	void DeleteFolderByPath(const FString& FolderPath);

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