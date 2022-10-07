// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPDependencyWalkerSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CPLog.h"
#include "CPSettings.h"
#include "EditorAssetLibrary.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/ProjectPackagingSettings.h"

#define LOCTEXT_NAMESPACE "CleanProject"

UCPDependencyWalkerSubsystem* UCPDependencyWalkerSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UCPDependencyWalkerSubsystem>();
}

namespace
{
FString GetTabSpaces(int TabSize)
{
	FString Result;
	for (int i = 0; i < TabSize; i++)
	{
		Result += TEXT("    ");
	}

	return Result;
}

void AddReferencesRecursive(const TArray<FAssetData>& Assets, const TMap<FAssetData, TArray<FAssetData>>& DependenciesTable,
	TSet<FAssetData>& OutReferences, int RecursionLevel = 0)
{
	for (const FAssetData& Asset : Assets)
	{
		UE_LOG(LogCleanProject, Log, TEXT("%s%s"), *GetTabSpaces(RecursionLevel), *Asset.ObjectPath.ToString());
		OutReferences.Add(Asset);

		TArray<FAssetData> AssetDependencies;
		for (TPair<FAssetData, TArray<FAssetData>> DependencyRow : DependenciesTable)
		{
			if (DependencyRow.Value.Contains(Asset))
			{
				AssetDependencies.Add(DependencyRow.Key);
			}
		}
		AssetDependencies.RemoveAll([=](const FAssetData& AssetData) { return OutReferences.Contains(AssetData); });

		AddReferencesRecursive(AssetDependencies, DependenciesTable, OutReferences, RecursionLevel + 1);
	}
}

TMap<FAssetData, TArray<FAssetData>> BuildDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType)
{
	FScopedSlowTask SlowTask(InAssets.Num(), LOCTEXT("BuildDependenciesTable", "Build Dependencies Table"));
	SlowTask.MakeDialog();
	double BuildDependenciesStart = FPlatformTime::Seconds();
	UE_LOG(LogCleanProject, Log, TEXT("Building dependencies started"));

	TMap<FAssetData, TArray<FAssetData>> Result;
	for (FAssetData Asset : InAssets)
	{
		const FString& AssetPath = Asset.ToSoftObjectPath().GetAssetPathString();

		SlowTask.EnterProgressFrame(1, FText::FromString(AssetPath));
		double AssetInspectingStart = FPlatformTime::Seconds();
		UE_LOG(LogCleanProject, Log, TEXT("Inspecting: %s started"), *AssetPath);

		const bool bLoadAssetsToConfirm = ScanType == EScanType::Complex;
		TArray<FString> Dependencies = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath, bLoadAssetsToConfirm);

		TArray<FAssetData> DependencyAssets;
		for (const FString& Dependency : Dependencies)
		{
			FAssetData DependencyAsset = UEditorAssetLibrary::FindAssetData(Dependency);
			DependencyAssets.Emplace(DependencyAsset);
		}
		Result.Emplace(Asset, DependencyAssets);

		double AssetInspectingDuration = FPlatformTime::Seconds() - AssetInspectingStart;
		UE_LOG(LogCleanProject, Log, TEXT("Inspecting: %s took %s seconds"), *AssetPath, *LexToString(AssetInspectingDuration));
	}

	double BuildDependenciesDuration = FPlatformTime::Seconds() - BuildDependenciesStart;
	UE_LOG(LogCleanProject, Log, TEXT("Building dependencies took: %s"), *LexToString(BuildDependenciesDuration));

	return Result;
}
}	 // namespace

void UCPDependencyWalkerSubsystem::CheckAllDependencies(EScanType ScanType)
{
	TArray<FAssetData> WhitelistedAssets = GetWhitelistedAssets();
	TSet<FAssetData> AllGameAssets = TSet(GetAllGameAssets());

	TMap<FAssetData, TArray<FAssetData>> DependenciesTable = BuildDependenciesTable(AllGameAssets, ScanType);

	TSet<FAssetData> AssetsToKeep;

	UE_LOG(LogCleanProject, Log, TEXT("*************** START RECURSIVE DEPENDENCIES ***************"))
	FScopedSlowTask SlowTask(1, LOCTEXT("RecursiveDependencies", "Recursive Dependencies"));
	SlowTask.MakeDialog();
	AddReferencesRecursive(WhitelistedAssets, DependenciesTable, AssetsToKeep);
	UE_LOG(LogCleanProject, Log, TEXT("*************** END RECURSIVE DEPENDENCIES ***************"))
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
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

	return AllAssetData;
}

TArray<FAssetData> UCPDependencyWalkerSubsystem::GetWhitelistedAssets() const
{
	TArray<FAssetData> Result;
	const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();

	// First get the assets which are explicitly cooked by the user via MapsToCook list
	Algo::Transform(PackagingSettings->MapsToCook, Result,
		[](const FFilePath& File) { return UEditorAssetLibrary::FindAssetData(File.FilePath); });

	// Second get the assets which are inside an always cook folder
	TArray<FString> FoldersToCook;
	Algo::Transform(
		PackagingSettings->DirectoriesToAlwaysCook, FoldersToCook, [](FDirectoryPath const& Directory) { return Directory.Path; });
	const TArray<FAssetData> AssetsToCook = GetAssetsInPaths(FoldersToCook);
	Algo::Transform(AssetsToCook, Result, [](const FAssetData& AssetData) { return AssetData; });

	// Third get the assets which were explicitly selected by the user in our plugin settings
	const TSet<FAssetData> ExplicitlyWhitelisted = GetDefault<UCPSettings>()->GetWhitelistAssetsPaths();
	Result.Append(ExplicitlyWhitelisted.Array());

	Result.RemoveAll([](const FAssetData& AssetData) { return !AssetData.IsValid(); });

	return Result;
}

TArray<FAssetData> UCPDependencyWalkerSubsystem::GetAllGameAssets() const
{
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AllAssetData);
	return AllAssetData;
}

void UCPDependencyWalkerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LOG_TRACE();
}

#undef LOCTEXT_NAMESPACE
