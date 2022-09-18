// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPDependencyWalkerSubsystem.h"

#include "CPLog.h"
#include "CPSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Settings/ProjectPackagingSettings.h"

UCPDependencyWalkerSubsystem* UCPDependencyWalkerSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UCPDependencyWalkerSubsystem>();
}

TArray<FAssetData> UCPDependencyWalkerSubsystem::GetAssetsInPaths(TArray<FString> FolderPaths) const
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

TSet<FName> UCPDependencyWalkerSubsystem::GetWhitelistedAssets() const
{
	TSet<FName> Result;
	const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();

	// First get the assets which are explicitly cooked by the user via MapsToCook list
	Algo::Transform(PackagingSettings->MapsToCook, Result, [](const FFilePath& File){ return FName(File.FilePath); });

	// Second get the assets which are inside an always cook folder
	TArray<FString> FoldersToCook;
	Algo::Transform(PackagingSettings->DirectoriesToAlwaysCook, FoldersToCook, [](FDirectoryPath const& Directory){ return Directory.Path; });
	const TArray<FAssetData> AssetsToCook = GetAssetsInPaths(FoldersToCook);
	Algo::Transform(AssetsToCook, Result, [](const FAssetData& AssetData){ return AssetData.ObjectPath; });

	// Third get the assets which were explicitly selected by the user in our plugin settings
	const TSet<FName> ExplicitlyWhitelisted = GetDefault<UCPSettings>()->GetWhitelistAssetsPaths();;
	Result.Append(ExplicitlyWhitelisted);

	return Result;
}

void UCPDependencyWalkerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LOG_TRACE();

	GetMutableDefault<UCPSettings>()->IncreaseSpaceGained(100);
}
