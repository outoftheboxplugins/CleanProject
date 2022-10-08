// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPDependencyWalkerSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CPLog.h"
#include "CPSettings.h"
#include "EditorAssetLibrary.h"
#include "Misc/ScopedSlowTask.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Widgets/SCPAssetDialog.h"

#define LOCTEXT_NAMESPACE "CleanProject"

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
}	 // namespace

FAssetDependenciesTable::FAssetDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType)
{
	BuildDependenciesTable(InAssets, ScanType);
}

void FAssetDependenciesTable::BuildDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType)
{
	UE_LOG(LogCleanProject, Log, TEXT("*************** START BuildDependenciesTable ***************"));
	FAutoScopedDurationTimer TotalTimer;

	FScopedSlowTask SlowTask(InAssets.Num(), LOCTEXT("BuildDependenciesTable", "Build Dependencies Table"));
	SlowTask.MakeDialog();

	for (FAssetData Asset : InAssets)
	{
		const FString& AssetPath = Asset.ToSoftObjectPath().GetAssetPathString();

		UE_LOG(LogCleanProject, Log, TEXT("Inspecting: %s started"), *AssetPath);
		FAutoScopedDurationTimer AssetTimer;

		SlowTask.EnterProgressFrame(1, FText::FromString(AssetPath));

		const bool bLoadAssetsToConfirm = ScanType == EScanType::Complex;
		TArray<FString> Dependencies = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath, bLoadAssetsToConfirm);

		TArray<FAssetData> DependencyAssets;
		for (const FString& Dependency : Dependencies)
		{
			FAssetData DependencyAsset = UEditorAssetLibrary::FindAssetData(Dependency);
			DependencyAssets.Emplace(DependencyAsset);
		}
		Table.Emplace(Asset, DependencyAssets);

		UE_LOG(LogCleanProject, Log, TEXT("Inspecting: %s took %s seconds"), *AssetPath, *LexToString(AssetTimer.GetTime()));
	}

	UE_LOG(LogCleanProject, Log, TEXT("Building dependencies took: %s"), *LexToString(TotalTimer.GetTime()));
	UE_LOG(LogCleanProject, Log, TEXT("*************** STOP BuildDependenciesTable ***************"));
}

TSet<FAssetData> FAssetDependenciesTable::CompileReferences(const TSet<FAssetData>& Assets)
{
	UE_LOG(LogCleanProject, Log, TEXT("*************** START CompileReferences ***************"));
	FAutoScopedDurationTimer TotalTimer;

	FScopedSlowTask SlowTask(1, LOCTEXT("RecursiveDependencies", "Recursive Dependencies"));
	SlowTask.MakeDialog();

	TSet<FAssetData> OutReferences;
	CompileReferencesRecursive(Assets.Array(), OutReferences);

	UE_LOG(LogCleanProject, Log, TEXT("Compiling references took: %s"), *LexToString(TotalTimer.GetTime()));
	UE_LOG(LogCleanProject, Log, TEXT("*************** END CompileReferences ***************"));

	return OutReferences;
}

void FAssetDependenciesTable::CompileReferencesRecursive(
	const TArray<FAssetData>& Assets, TSet<FAssetData>& OutReferences, int RecursionLevel /* = 0 */)
{
	for (const FAssetData& Asset : Assets)
	{
		UE_LOG(LogCleanProject, Log, TEXT("%s%s"), *GetTabSpaces(RecursionLevel), *Asset.ObjectPath.ToString());
		OutReferences.Add(Asset);

		TArray<FAssetData> AssetDependencies;
		for (TPair<FAssetData, TArray<FAssetData>> DependencyRow : Table)
		{
			if (DependencyRow.Value.Contains(Asset))
			{
				AssetDependencies.Add(DependencyRow.Key);
			}
		}
		AssetDependencies.RemoveAll([=](const FAssetData& AssetData) { return OutReferences.Contains(AssetData); });

		CompileReferencesRecursive(AssetDependencies, OutReferences, RecursionLevel + 1);
	}
}

UCPDependencyWalkerSubsystem* UCPDependencyWalkerSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UCPDependencyWalkerSubsystem>();
}

void UCPDependencyWalkerSubsystem::CheckAllDependencies(EScanType ScanType)
{
	const TArray<FAssetData> AssetsToCheck = GetAllGameAssets().Array();
	CheckDependenciesOf(AssetsToCheck, ScanType);
}

void UCPDependencyWalkerSubsystem::CheckDependenciesOf(const TArray<FString>& InFolders, EScanType ScanType)
{
	const TArray<FAssetData> AssetsInSelectedFolders = GetAssetsInPaths(InFolders);
	CheckDependenciesOf(AssetsInSelectedFolders, ScanType);
}

void UCPDependencyWalkerSubsystem::CheckDependenciesOf(const TArray<FAssetData>& InAssets, EScanType ScanType)
{
	if (ScanType == EScanType::Complex)
	{
		UE_LOG(LogCleanProject, Error,
			TEXT("Complex scan is currently not available due to a crash while unloading in "
				 "UEditorAssetLibrary::FindPackageReferencersForAsset"))
	}
	const TSet<FAssetData> WhitelistedAssets = GetWhitelistedAssets();

	FAssetDependenciesTable DependenciesTable = FAssetDependenciesTable(GetAllGameAssets(), ScanType);
	const TSet<FAssetData> AssetsToKeep = DependenciesTable.CompileReferences(WhitelistedAssets);

	TArray<FAssetData> AssetsToRemove = InAssets;
	AssetsToRemove.RemoveAll([=](const FAssetData& AssetData) { return AssetsToKeep.Contains(AssetData); });

	if (AssetsToRemove.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilesToDelete", "No unused assets found."));
	}
	else
	{
		SCPAssetDialog::OpenAssetDialog(AssetsToRemove);
	}
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

TSet<FAssetData> UCPDependencyWalkerSubsystem::GetWhitelistedAssets() const
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

	// Finally, remove all invalid assets before returning back
	Result.RemoveAll([](const FAssetData& AssetData) { return !AssetData.IsValid(); });

	return TSet(Result);
}

TSet<FAssetData> UCPDependencyWalkerSubsystem::GetAllGameAssets() const
{
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AllAssetData);
	return TSet(AllAssetData);
}

#undef LOCTEXT_NAMESPACE
