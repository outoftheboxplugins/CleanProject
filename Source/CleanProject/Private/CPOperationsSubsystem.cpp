// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPOperationsSubsystem.h"

#include "CPLog.h"
#include "CPSettings.h"
#include "Widgets/SCPUnusedAssetsReport.h"

#include <AssetRegistry/AssetRegistryModule.h>
#include <AssetToolsModule.h>
#include <AssetViewUtils.h>
#include <EditorAssetLibrary.h>
#include <Misc/ScopedSlowTask.h>
#include <ProfilingDebugging/ScopedTimers.h>
#include <Settings/ProjectPackagingSettings.h>

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

void GetEmptyFolderInPath(const FString& BaseDirectory, TArray<FString>& OutEmptyFolders)
{
	struct FEmptyFolderVisitor : public IPlatformFile::FDirectoryVisitor
	{
		TArray<FString>& EmptyFolders;
		const FString& CurrentDirectory;
		bool bIsEmpty;

		FEmptyFolderVisitor(TArray<FString>& InEmptyFolders, const FString& InCurrentDirectory)
			: EmptyFolders(InEmptyFolders), CurrentDirectory(InCurrentDirectory), bIsEmpty(true)
		{
		}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (bIsDirectory)
			{
				const FString DirectoryName(FilenameOrDirectory);
				EmptyFolders.Add(DirectoryName);

				GetEmptyFolderInPath(DirectoryName, EmptyFolders);
			}
			else
			{
				EmptyFolders.Remove(CurrentDirectory);
				bIsEmpty = false;
			}

			return true;
		}
	};
	FEmptyFolderVisitor EmptyFolderVisitor(OutEmptyFolders, BaseDirectory);
	IFileManager::Get().IterateDirectoryRecursively(*BaseDirectory, EmptyFolderVisitor);
	if (EmptyFolderVisitor.bIsEmpty)
	{
		OutEmptyFolders.Add(BaseDirectory);
	}
}
}	 // namespace

FCPAssetDependenciesTable::FAssetDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType)
{
	BuildDependenciesTable(InAssets, ScanType);
}

void FCPAssetDependenciesTable::BuildDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType)
{
	UE_LOG(LogCleanProject, VeryVerbose, TEXT("*************** START BuildDependenciesTable ***************"));
	FAutoScopedDurationTimer TotalTimer;

	FScopedSlowTask SlowTask(InAssets.Num(), LOCTEXT("BuildDependenciesTable", "Build Dependencies Table"));
	SlowTask.MakeDialog();

	for (FAssetData Asset : InAssets)
	{
		const FString& AssetPath = Asset.ToSoftObjectPath().GetAssetPathString();

		UE_LOG(LogCleanProject, VeryVerbose, TEXT("Inspecting: %s started"), *AssetPath);
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

		UE_LOG(
			LogCleanProject, VeryVerbose, TEXT("Inspecting: %s took %s seconds"), *AssetPath, *LexToString(AssetTimer.GetTime()));
	}

	UE_LOG(LogCleanProject, VeryVerbose, TEXT("Building dependencies took: %s"), *LexToString(TotalTimer.GetTime()));
	UE_LOG(LogCleanProject, VeryVerbose, TEXT("*************** STOP BuildDependenciesTable ***************"));
}

TSet<FAssetData> FCPAssetDependenciesTable::CompileReferences(const TSet<FAssetData>& Assets)
{
	UE_LOG(LogCleanProject, VeryVerbose, TEXT("*************** START CompileReferences ***************"));
	FAutoScopedDurationTimer TotalTimer;

	FScopedSlowTask SlowTask(1, LOCTEXT("RecursiveDependencies", "Recursive Dependencies"));
	SlowTask.MakeDialog();

	TSet<FAssetData> OutReferences;
	CompileReferencesRecursive(Assets.Array(), OutReferences);

	UE_LOG(LogCleanProject, VeryVerbose, TEXT("Compiling references took: %s"), *LexToString(TotalTimer.GetTime()));
	UE_LOG(LogCleanProject, VeryVerbose, TEXT("*************** END CompileReferences ***************"));

	return OutReferences;
}

void FCPAssetDependenciesTable::CompileReferencesRecursive(
	const TArray<FAssetData>& Assets, TSet<FAssetData>& OutReferences, int RecursionLevel /* = 0 */)
{
	for (const FAssetData& Asset : Assets)
	{
		UE_LOG(LogCleanProject, VeryVerbose, TEXT("%s%s"), *GetTabSpaces(RecursionLevel), *Asset.ObjectPath.ToString());
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

UCPOperationsSubsystem* UCPOperationsSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UCPOperationsSubsystem>();
}

void UCPOperationsSubsystem::DeleteAllUnusedAssets(EScanType ScanType)
{
	const TArray<FAssetData> AllAssets = GetAllGameAssets().Array();
	DeleteUnusedAssets(AllAssets, ScanType);
}

void UCPOperationsSubsystem::DeleteUnusedAssets(const TArray<FString>& InFolders, EScanType ScanType)
{
	const TArray<FAssetData> AssetsInSelectedFolders = GetAssetsInPaths(InFolders);
	DeleteUnusedAssets(AssetsInSelectedFolders, ScanType);
}

void UCPOperationsSubsystem::DeleteUnusedAssets(const TArray<FAssetData>& InAssets, EScanType ScanType)
{
	const TArray<FAssetData> AssetsToRemove = GetUnusedAssets(InAssets, ScanType);
	if (AssetsToRemove.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilesToDelete", "No unused assets found."));
	}
	else
	{
		SCPUnusedAssetsReport::OpenAssetDialog(AssetsToRemove);
	}
}

void UCPOperationsSubsystem::DeleteAllEmptyPackageFolders()
{
	FString GameContentFolder = TEXT("/Game");
	DeleteEmptyPackageFoldersIn({GameContentFolder});
}

void UCPOperationsSubsystem::DeleteEmptyPackageFoldersIn(const TArray<FString>& InPaths)
{
	for (const FString& Path : InPaths)
	{
		DeleteEmptyPackageFoldersIn(Path);
	}
}

void UCPOperationsSubsystem::DeleteEmptyPackageFoldersIn(const FString& InPath)
{
	FixUpRedirectsInProject();

	const FString& FullPathFolder = FPackageName::LongPackageNameToFilename(InPath);
	if (FullPathFolder.IsEmpty())
	{
		UE_LOG(LogCleanProject, Warning, TEXT("Could not convert folder: %s to full path"), *InPath);
		return;
	}

	TArray<FString> ResultEmptyFolders;
	GetEmptyFolderInPath(FullPathFolder, ResultEmptyFolders);

	for (const FString& FolderToDelete : ResultEmptyFolders)
	{
		IFileManager::Get().DeleteDirectory(*FolderToDelete, false, true);
	}
}

void UCPOperationsSubsystem::FixUpRedirectsInProject()
{
	const TSet<FAssetData> RedirectorAssets = GetAllGameAssetsOfType<UObjectRedirector>();

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : RedirectorAssets)
	{
		ObjectPaths.Add(Asset.ObjectPath.ToString());
	}

	TArray<UObject*> Objects;
	if (AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, true, true))
	{
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			UObjectRedirector* Redirector = CastChecked<UObjectRedirector>(Object);
			Redirectors.Add(Redirector);
		}

		FAssetToolsModule::GetModule().Get().FixupReferencers(Redirectors);
	}
}

TArray<FAssetData> UCPOperationsSubsystem::GetAllUnusedAssets(EScanType ScanType) const
{
	const TArray<FAssetData> AllAssets = GetAllGameAssets().Array();
	return GetUnusedAssets(AllAssets, ScanType);
}

TArray<FAssetData> UCPOperationsSubsystem::GetUnusedAssets(const TArray<FAssetData>& AssetsToCheck, EScanType ScanType) const
{
	if (ScanType == EScanType::Complex)
	{
		UE_LOG(LogCleanProject, Error,
			TEXT("Complex scan is currently not available due to a crash while unloading in "
				 "UEditorAssetLibrary::FindPackageReferencersForAsset"));

		return {};
	}
	const TSet<FAssetData> WhitelistedAssets = GetWhitelistedAssets();

	FCPAssetDependenciesTable DependenciesTable = FCPAssetDependenciesTable(GetAllGameAssets(), ScanType);
	const TSet<FAssetData> AssetsToKeep = DependenciesTable.CompileReferences(WhitelistedAssets);

	TArray<FAssetData> UnusedAssets = AssetsToCheck;
	UnusedAssets.RemoveAll([=](const FAssetData& AssetData) { return AssetsToKeep.Contains(AssetData); });

	return UnusedAssets;
}

TArray<FAssetData> UCPOperationsSubsystem::GetAssetsInPaths(TArray<FString> FolderPaths) const
{
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	for (const FString& FolderPath : FolderPaths)
	{
		Filter.PackagePaths.Add(FName(FolderPath));
	}
	TArray<FAssetData> AllAssetData;
	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AllAssetData);

	return AllAssetData;
}

TSet<FAssetData> UCPOperationsSubsystem::GetWhitelistedAssets() const
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

TSet<FAssetData> UCPOperationsSubsystem::GetAllGameAssets(TOptional<FName> ClassFilter) const
{
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	if (ClassFilter.IsSet())
	{
		Filter.ClassNames.Add(ClassFilter.GetValue());
	}

	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AllAssetData);
	return TSet(AllAssetData);
}

#undef LOCTEXT_NAMESPACE
