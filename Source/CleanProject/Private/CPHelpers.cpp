// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPHelpers.h"

#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

TSet<FAssetData> CPHelpers::GetAllGameAssets(TOptional<FTopLevelAssetPath> ClassFilter)
{
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	if (ClassFilter.IsSet())
	{
		Filter.ClassPaths.Add(ClassFilter.GetValue());
	}

	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AllAssetData);
	return TSet(AllAssetData);
}

TArray<FAssetData> CPHelpers::GetAssetsInPaths(TArray<FString> FolderPaths)
{
	TArray<FAssetData> AllAssetData;
	AssetViewUtils::GetAssetsInPaths(FolderPaths, AllAssetData);

	return AllAssetData;
}

TArray<FAssetData> CPHelpers::GetAssetsInPaths(FString FolderPath)
{
	const TArray<FString> FolderPaths = {FolderPath};
	return GetAssetsInPaths(FolderPaths);
}
