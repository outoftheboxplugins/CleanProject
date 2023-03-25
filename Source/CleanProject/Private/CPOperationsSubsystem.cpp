// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPOperationsSubsystem.h"

#include <AssetToolsModule.h>
#include <AssetViewUtils.h>
#include <EditorAssetLibrary.h>
#include <GameDelegates.h>
#include <Misc/ScopedSlowTask.h>
#include <ProfilingDebugging/ScopedTimers.h>
#include <Settings/ProjectPackagingSettings.h>

#include "CPHelpers.h"
#include "CPLog.h"
#include "CPSettings.h"
#include "Widgets/SCPUnusedAssetsReport.h"

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

			FEmptyFolderVisitor(TArray<FString>& InEmptyFolders, const FString& InCurrentDirectory) : EmptyFolders(InEmptyFolders), CurrentDirectory(InCurrentDirectory), bIsEmpty(true) {}

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
} // namespace

FCPAssetDependenciesTable::FCPAssetDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType)
{
	BuildDependenciesTable(InAssets, ScanType);
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

		UE_LOG(LogCleanProject, VeryVerbose, TEXT("Inspecting: %s took %s seconds"), *AssetPath, *LexToString(AssetTimer.GetTime()));
	}

	UE_LOG(LogCleanProject, VeryVerbose, TEXT("Building dependencies took: %s"), *LexToString(TotalTimer.GetTime()));
	UE_LOG(LogCleanProject, VeryVerbose, TEXT("*************** STOP BuildDependenciesTable ***************"));
}

void FCPAssetDependenciesTable::CompileReferencesRecursive(const TArray<FAssetData>& Assets, TSet<FAssetData>& OutReferences, int RecursionLevel /* = 0 */)
{
	for (const FAssetData& Asset : Assets)
	{
		UE_LOG(LogCleanProject, VeryVerbose, TEXT("%s%s"), *GetTabSpaces(RecursionLevel), *Asset.GetSoftObjectPath().ToString());
		OutReferences.Add(Asset);

		TArray<FAssetData> AssetDependencies;
		for (TPair<FAssetData, TArray<FAssetData>> DependencyRow : Table)
		{
			if (DependencyRow.Value.Contains(Asset))
			{
				AssetDependencies.Add(DependencyRow.Key);
			}
		}
		AssetDependencies.RemoveAll(
			[=](const FAssetData& AssetData)
			{
				return OutReferences.Contains(AssetData);
			}
		);

		CompileReferencesRecursive(AssetDependencies, OutReferences, RecursionLevel + 1);
	}
}

UCPOperationsSubsystem* UCPOperationsSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UCPOperationsSubsystem>();
}

void UCPOperationsSubsystem::DeleteAllUnusedAssets(EScanType ScanType)
{
	const TArray<FAssetData> AllAssets = CPHelpers::GetAllGameAssets().Array();
	DeleteUnusedAssets(AllAssets, ScanType);
}

void UCPOperationsSubsystem::DeleteUnusedAssets(const TArray<FString>& InFolders, EScanType ScanType)
{
	const TArray<FAssetData> AssetsInSelectedFolders = CPHelpers::GetAssetsInPaths(InFolders);
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
		SCPUnusedAssetsReport::OpenDialog(AssetsToRemove);
	}
}

void UCPOperationsSubsystem::DeleteAllEmptyFolders()
{
	FString GameContentFolder = TEXT("/Game");
	DeleteEmptyFoldersIn({GameContentFolder});
}

void UCPOperationsSubsystem::DeleteEmptyFoldersIn(const TArray<FString>& InPaths)
{
	for (const FString& Path : InPaths)
	{
		DeleteEmptyFoldersIn(Path);
	}
}

void UCPOperationsSubsystem::DeleteEmptyFoldersIn(const FString& InPath)
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
	const TSet<FAssetData> RedirectsAssets = CPHelpers::GetAllGameAssetsOfType<UObjectRedirector>();

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : RedirectsAssets)
	{
		ObjectPaths.Add(Asset.GetSoftObjectPath().ToString());
	}

	TArray<UObject*> Objects;
	if (AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, true, true))
	{
		TArray<UObjectRedirector*> Redirects;
		for (UObject* Object : Objects)
		{
			UObjectRedirector* Redirect = CastChecked<UObjectRedirector>(Object);
			Redirects.Add(Redirect);
		}

		FAssetToolsModule::GetModule().Get().FixupReferencers(Redirects);
	}
}

TArray<FAssetData> UCPOperationsSubsystem::GetAllUnusedAssets(EScanType ScanType) const
{
	const TArray<FAssetData> AllAssets = CPHelpers::GetAllGameAssets().Array();
	return GetUnusedAssets(AllAssets, ScanType);
}

TArray<FAssetData> UCPOperationsSubsystem::GetUnusedAssets(const TArray<FAssetData>& AssetsToCheck, EScanType ScanType) const
{
	if (ScanType == EScanType::Complex)
	{
		UE_LOG(LogCleanProject, Error, TEXT("Complex scan is currently not available due to a crash while unloading in UEditorAssetLibrary::FindPackageReferencersForAsset"));
		return {};
	}
	const TSet<FAssetData> CoreAssets = GetAllCoreAssets();

	FCPAssetDependenciesTable DependenciesTable = FCPAssetDependenciesTable(CPHelpers::GetAllGameAssets(), ScanType);
	const TSet<FAssetData> AssetsToKeep = DependenciesTable.CompileReferences(CoreAssets);

	TArray<FAssetData> UnusedAssets = AssetsToCheck;
	UnusedAssets.RemoveAll(
		[=](const FAssetData& AssetData)
		{
			return AssetsToKeep.Contains(AssetData);
		}
	);

	return UnusedAssets;
}

TSet<FAssetData> UCPOperationsSubsystem::GetAllCoreAssets() const
{
	TArray<FAssetData> Result;
	const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();

	// First get the assets which are explicitly cooked by the user via MapsToCook list
	Algo::Transform(
		PackagingSettings->MapsToCook,
		Result,
		[](const FFilePath& File)
		{
			return UEditorAssetLibrary::FindAssetData(File.FilePath);
		}
	);

	// Second get the assets which are inside an always cook folder
	TArray<FString> FoldersToCook;
	Algo::Transform(
		PackagingSettings->DirectoriesToAlwaysCook,
		FoldersToCook,
		[](FDirectoryPath const& Directory)
		{
			return Directory.Path;
		}
	);
	const TArray<FAssetData> AssetsToCook = CPHelpers::GetAssetsInPaths(FoldersToCook);
	Algo::Transform(
		AssetsToCook,
		Result,
		[](const FAssetData& AssetData)
		{
			return AssetData;
		}
	);

	// Third get the assets which were explicitly selected by the user in our plugin settings
	const TSet<FAssetData> ExplicitCoreAssets = GetDefault<UCPSettings>()->GetCoreAssets();
	Result.Append(ExplicitCoreAssets.Array());

	// Forth get the assets referenced inside the .ini Settings
	Result.Append(CPHelpers::GetAssetsInIniFiles());

	// Finally, remove all invalid assets before returning back
	Result.RemoveAll(
		[](const FAssetData& AssetData)
		{
			return !AssetData.IsValid();
		}
	);

	return TSet(Result);
}

void UCPOperationsSubsystem::ModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook)
{
	UE_LOG(LogCleanProject, Display, TEXT("Modifying content to be cooked"));

	const TSet<FAssetData> AssetsExcludedFromPackage = GetDefault<UCPSettings>()->GetAssetsExcludedFromPackage();
	for (const FAssetData& ExcludedAsset : AssetsExcludedFromPackage)
	{
		UE_LOG(LogCleanProject, Display, TEXT("%s is excluded from packaging and will not be included in the final build"), *ExcludedAsset.GetObjectPathString());
		InOutPackagesToNeverCook.Add(ExcludedAsset.PackageName);
	}
}

void UCPOperationsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FModifyCookDelegate& ModifyCookDelegate = FGameDelegates::Get().GetModifyCookDelegate();
	ModifyCookDelegate.AddUObject(this, &ThisClass::ModifyCook);
}

#undef LOCTEXT_NAMESPACE