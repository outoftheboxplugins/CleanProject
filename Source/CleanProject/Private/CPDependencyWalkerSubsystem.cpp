// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPDependencyWalkerSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"

UCPDependencyWalkerSubsystem* UCPDependencyWalkerSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UCPDependencyWalkerSubsystem>();
}

TArray<FAssetData> UCPDependencyWalkerSubsystem::GetAssetsInPaths(TArray<FString> FolderPaths)
{
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	for (const FString& FolderPath : FolderPaths)
	{
		Filter.PackagePaths.Add(FName(FolderPath));
	}
	TArray<FAssetData> AllAssetData;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

	return AllAssetData;
}
