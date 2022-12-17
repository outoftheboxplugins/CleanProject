// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <EditorSubsystem.h>

#include "CPOperationsSubsystem.generated.h"

/**
 * @brief Mechanics used to build the dependency table
 */
enum class EScanType
{
	Fast,
	// used cached references without loading the object in memory
	Complex // load the object in memory to get up to date references
};

/**
 * @brief Builds a dependencies table based on the input assets and scan type
 */
struct CLEANPROJECT_API FCPAssetDependenciesTable
{
	/**
	 * @brief Constructs and caches the dependencies table so we can perform lookups with it
	 * @param InAssets All assets we want to check references for
	 * @param ScanType Mechanics used to determine the references of the input assets
	 */
	FCPAssetDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType);
	/**
	 * @brief Computes a list of all the references of the input assets (including children references) based on the cached table
	 * @param Assets Assets we want to get all references for
	 * @return All the assets referenced by the input assets
	 */
	TSet<FAssetData> CompileReferences(const TSet<FAssetData>& Assets);

private:
	/**
	 * @brief Constructs and caches the dependencies table so we can perform lookups with it
	 * @param InAssets All assets we want to check references for
	 * @param ScanType Mechanics used to determine the references of the input assets
	 */
	void BuildDependenciesTable(const TSet<FAssetData>& InAssets, EScanType ScanType);
	/**
	 * @brief Recursively compiles the list of all references of the input assets
	 * @param Assets Assets we want to check inside our dependencies table and get references
	 * @param OutReferences Output parameter where we append references of the input assets
	 * @param RecursionLevel Depth of recursion reached
	 */
	void CompileReferencesRecursive(const TArray<FAssetData>& Assets, TSet<FAssetData>& OutReferences, int RecursionLevel = 0);
	/**
	 * @brief Cached Map between Asset and all the assets that references it
	 */
	TMap<FAssetData, TArray<FAssetData>> Table;
};

/**
 * @brief Entry point to start any Clean Project operation (e.g.: Deleting unused assets, deleting empty folder)
 */
UCLASS()
class CLEANPROJECT_API UCPOperationsSubsystem final : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Static wrapper for getting this engine subsystem. Will return nullptr if not loaded yet
	 */
	static UCPOperationsSubsystem* Get();

	/**
	 * @brief Checks for any unused assets inside the Game's content folder
	 * @param ScanType Method used to determine references
	 */
	void DeleteAllUnusedAssets(EScanType ScanType);
	/**
	 * @brief Checks for any unused assets inside the input folders list
	 * @param InFolders Folders to check
	 * @param ScanType Method used to determine references
	 */
	void DeleteUnusedAssets(const TArray<FString>& InFolders, EScanType ScanType);
	/**
	 * @brief Checks for any unused assets inside the input asset list
	 * @param InAssets Assets to check
	 * @param ScanType Method used to determine references
	 */
	void DeleteUnusedAssets(const TArray<FAssetData>& InAssets, EScanType ScanType);

	/**
	 * @brief Delete all empty folder inside the Game's content folder
	 * @note Fixes up Redirects before perform action @see FixUpRedirectsInProject
	 */
	void DeleteAllEmptyPackageFolders();
	/**
	 * @brief Delete all empty folders inside the input folders
	 * @param InPaths Folders to check
	 * @note Fixes up Redirects before perform action @see FixUpRedirectsInProject
	 */
	void DeleteEmptyPackageFoldersIn(const TArray<FString>& InPaths);
	/**
	 * @brief Delete all empty folders inside the input folder
	 * @param InPath Folder to check
	 * @note Fixes up Redirects before perform action @see FixUpRedirectsInProject
	 */
	void DeleteEmptyPackageFoldersIn(const FString& InPath);

	/**
	 * @brief Fixes up Redirects inside the Game's content folder
	 */
	void FixUpRedirectsInProject();

	/**
	 * @brief Determine all unused assets from all assets inside the Game's content folder
	 * @param ScanType Method used to determine references
	 * @return List of unused assets
	 */
	TArray<FAssetData> GetAllUnusedAssets(EScanType ScanType) const;
	/**
	 * @brief Determine all unused assets from the input assets
	 * @param AssetsToCheck Assets we want to check if they are unused
	 * @param ScanType Method used to determine references
	 * @return List of unused assets
	 */
	TArray<FAssetData> GetUnusedAssets(const TArray<FAssetData>& AssetsToCheck, EScanType ScanType) const;

	/**
	 * @brief Recursively gets all the assets from the input folders
	 * @param FolderPaths Folders we want to recursively get all assets from
	 * @return List of all the assets found inside the input Folders
	 */
	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths) const;
	TArray<FAssetData> GetAssetsInPaths(FString FolderPath) const;

	/**
	 * @brief Gets all the assets of the input type from the Game's content folder
	 * @tparam T Type of asset we want to to look for
	 * @return List of all the assets found
	 */
	template <typename T>
	TSet<FAssetData> GetAllGameAssetsOfType() const;
	/**
	 * @brief Gets all the assets from the Game's content folder. Optionally filters them by a class type
	 * @param ClassFilter Class we want to filter assets for. If unset no filtering will be done and assets will be returned
	 * @return List of all the assets found
	 */
	TSet<FAssetData> GetAllGameAssets(TOptional<FTopLevelAssetPath> ClassFilter = {}) const;

	/**
	 * @brief Computes a set of all the whitelisted assets, takes into account:
	 * 1. Maps added in MapsToCook Project settings
	 * 2. Directories added in DirectoriesToAlwaysCook Project settings
	 * 3. Default Game Objects inside Project settings
	 * 4. Assets added inside Clean Project settings
	 */
	TSet<FAssetData> GetWhitelistedAssets() const;

private:
	FAssetData GetDefaultGameObject(const FName& PropertyName) const;
};

template <typename T>
TSet<FAssetData> UCPOperationsSubsystem::GetAllGameAssetsOfType() const
{
	return GetAllGameAssets(T::StaticClass()->GetClassPathName());
}
