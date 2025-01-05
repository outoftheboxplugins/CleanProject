// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <EditorSubsystem.h>

#include "CPOperationsSubsystem.generated.h"

/**
 * @brief Mechanics used to build the dependency table
 */
enum class EScanType
{
	Fast, // uses cached references without loading the object in memory
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
	void DeleteAllEmptyFolders();
	/**
	 * @brief Delete all empty folders inside the input folders
	 * @param InPaths Folders to check
	 * @note Fixes up Redirects before perform action @see FixUpRedirectsInProject
	 */
	void DeleteEmptyFoldersIn(const TArray<FString>& InPaths);
	/**
	 * @brief Delete all empty folders inside the input folder
	 * @param InPath Folder to check
	 * @note Fixes up Redirects before perform action @see FixUpRedirectsInProject
	 */
	void DeleteEmptyFoldersIn(const FString& InPath);
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
	 * @brief Computes a set of all the core assets, takes into account:
	 * 1. Maps added in MapsToCook Project settings
	 * 2. Directories added in DirectoriesToAlwaysCook Project settings
	 * 3. Default Game Objects inside Project settings
	 * 4. Assets added inside Clean Project settings
	 */
	TSet<FAssetData> GetAllCoreAssets() const;

private:
	/**
	 * @brief Delegate to modify cooking behavior - can add extra packages to cook or prevent certain packages from cooking
	 * @param InTargetPlatforms Target platforms we are currently cooking for
	 * @param InOutPackagesToCook Add packages to this list to cook them
	 * @param InOutPackagesToNeverCook Add packages to this list to prevent them from cooking
	 */
	void ModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook);

	// Begin UEditorSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End UEditorSubsystem interface
};
