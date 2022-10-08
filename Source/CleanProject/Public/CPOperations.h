// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "AssetData.h"
#include "CoreMinimal.h"

using FAssetDataPtr = TSharedPtr<FName>;

namespace CPOperations
{
int64 GetAssetDiskSize(const FAssetData& Asset);
int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList);

struct FChildDependency
{
	explicit FChildDependency(const FName& InAssetName);

	bool operator!=(const FChildDependency& Other) const;

	void AddDependency(const FName& ChildDependency);
	bool HasChildren() const
	{
		return ChildDependencies.Num() != 0;
	}

	FName GetAssetName() const
	{
		return *AssetName;
	}
	TArray<FAssetDataPtr> GetChildrenAssetPtrs();

	FAssetDataPtr AssetName;
	TArray<FChildDependency> ChildDependencies;
};

struct FTreeAssetDependency
{
	operator TArray<FName>() const
	{
		return GetDependenciesAsList();
	}
	FChildDependency& operator[](const FName& OwnerName)
	{
		return GetDependencyRecursive(OwnerName, TopLevelDependencies);
	}

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
void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, FTreeAssetDependency& ResultTreeDependency);

// Delete all the empty folders of the project.
void DeleteEmptyProjectFolders();
void DeleteEmptyProjectFolders(TArray<FString> SelectedFolders);

void DeleteFolderByPath(const FString& FolderPath);
};	  // namespace CPOperations
