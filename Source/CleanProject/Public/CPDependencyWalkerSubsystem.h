// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"

#include "CPDependencyWalkerSubsystem.generated.h"

enum class EScanType
{
	Fast,
	Complex
};

struct FAssetDependenciesTable
{
	FAssetDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType);

	TSet<FAssetData> CompileReferences(const TSet<FAssetData>& Assets);

private:
	void BuildDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType);
	void CompileReferencesRecursive(const TArray<FAssetData>& Assets, TSet<FAssetData>& OutReferences, int RecursionLevel = 0);

	TMap<FAssetData, TArray<FAssetData>> Table;
};

UCLASS()
class CLEANPROJECT_API UCPDependencyWalkerSubsystem final : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static UCPDependencyWalkerSubsystem* Get();

	void DeleteAllUnusedAssets(EScanType ScanType);
	void DeleteUnusedAssets(const TArray<FString>& InFolders, EScanType ScanType);
	void DeleteUnusedAssets(const TArray<FAssetData>& InAssets, EScanType ScanType);

	void FixUpRedirectsInProject();

	TArray<FAssetData> GetAllUnusedAssets(EScanType ScanType) const;
	TArray<FAssetData> GetUnusedAssets(const TArray<FAssetData>& AssetsToCheck, EScanType ScanType) const;
	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths) const;

private:
	TSet<FAssetData> GetWhitelistedAssets() const;
	TSet<FAssetData> GetAllGameAssets(TOptional<FName> ClassFilter = {}) const;

	template <typename T>
	TSet<FAssetData> GetAllGameAssetsOfType() const;
};

template <typename T>
TSet<FAssetData> UCPDependencyWalkerSubsystem::GetAllGameAssetsOfType() const
{
	return GetAllGameAssets(T::StaticClass()->GetFName());
}
