// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "AssetData.h"
#include "CoreMinimal.h"
#include "Engine/World.h"

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

// Fix up the redirects in the whole project.
void FixUpRedirectsInProject();

// Delete all the empty folders of the project.
void DeleteEmptyProjectFolders();
void DeleteEmptyProjectFolders(TArray<FString> SelectedFolders);

void DeleteFolderByPath(const FString& FolderPath);

// Returns all the assets from the project (Game folder).
template <typename T>
TArray<FAssetData> GetAllGameAssets();
TArray<FAssetData> GetAllGameAssets(TArray<FName> ClassTypes = TArray<FName>());
};	  // namespace CPOperations

template <typename T>
TArray<FAssetData> CPOperations::GetAllGameAssets()
{
	TArray<FName> MapsClassFilter;
	MapsClassFilter.Add(T::StaticClass()->GetFName());

	return CPOperations::GetAllGameAssets(MapsClassFilter);
}
