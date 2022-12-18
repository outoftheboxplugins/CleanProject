// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <Engine/DeveloperSettings.h>

#include "CPSettings.generated.h"

// TODO: Find a decent way to clean up assets and folders paths that are no longer valid and avoid duplicated entries
// TODO: Can we make the properties of this settings class private and just expose some getters?
// TODO: Can we remove the reference to the operations subsystem and throw the getters in there?
// TODO: Check where LOG_TRACE is used and determine where we should add it next, maybe inside all the operations of CPOperations ?

/**
 * @brief Holds the configurable settings for the Clean Project plugin
 */
UCLASS(config = Editor, defaultconfig)
class UCPSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * @brief Shortcut to open the project settings windows focused to this config
	 */
	static void OpenSettings();
	/**
	 * @brief Called after this settings object has been changed
	 */
	FSimpleMulticastDelegate OnSettingsChanged;
	/**
	 * @brief List of assets we always consider actively referenced.
	 * Assets referenced (and their dependencies) will be deleted by our system
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Cleanup settings")
	TArray<FSoftObjectPath> CoreAssets;
	/**
	 * @brief List of folders we always consider actively referenced.
	 * Assets inside the referenced folders (and their dependencies) will be deleted by our system
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Cleanup settings", meta = (LongPackageName))
	TArray<FDirectoryPath> CoreFolders;
	/**
	 * @brief Old version for storing assets we want to whitelist
	 */
	UE_DEPRECATED(5.0, "WhitelistAssetsPaths has been removed as a way of storing references. Please use ")
	UPROPERTY(VisibleDefaultsOnly, config, Category = "Deprecated")
	TArray<FString> WhitelistAssetsPaths;
	/**
	 * TODO
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Cleanup settings")
	TArray<FSoftObjectPath> AssetsExcludedFromPackage;
	/**
	 * TODO
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Cleanup settings", meta = (LongPackageName))
	TArray<FDirectoryPath> FoldersExcludedFromPackage;
	/**
	 * @brief  Should the Clean Project Dashboard automatically refresh itself every time an asset is updated. (Unless time since
	 * last refresh is lower than AutoRefreshInterval)
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Cleanup settings")
	bool bAutoRefreshDashboard = true;
	/**
	 * @brief How much time we should wait in between Clean Project Dashboard refresh requests
	 * @note measured in seconds
	 * @note use -1 to refresh every time
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Cleanup settings")
	float AutoRefreshInterval = 30.0f;
	/**
	 * @brief Convenience function to programatically add assets to the core list and saving config
	 */
	void MarkAssetsAsCore(const TArray<FAssetData> Assets);
	/**
	 * @brief Convenience function to programatically add folders to the core list and saving config
	 */
	void MarkPathsAsCore(const TArray<FString> Paths);
	/**
	 * @brief Convenience function to programatically add assets to the package exclusion list and saving config
	 */
	void ExcludeAssetsFromPackage(const TArray<FAssetData> Assets);
	/**
	 * @brief Convenience function to programatically add folders to the package exclusion list and saving config
	 */
	void ExcludePathsFromPackage(const TArray<FString> Paths);
	// TODO: Unify GetWhitelistedAssetsPaths and GetBlacklistedAssetsPaths in a common getter with functionality
	/**
	 * @brief Returns the asset path of the assets explicitly marked as Core
	 */
	TSet<FAssetData> GetCoreAssets() const;
	/**
	 * @brief Returns the asset path of the assets excluded from package
	 */
	TSet<FAssetData> GetAssetsExcludedFromPackage() const;

private:
	// Begin UDeveloperSettings interface
	virtual void PostInitProperties() override;
	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
	// End UDeveloperSettings interface

	/**
	 * @brief Saves the current values to the .ini file inside root Config folder
	 */
	void SaveToDefaultConfig();
	/**
	 * @brief Callback executed when any config is saved so we can react to saves to this config
	 */
	void OnAnyConfigSaved(const TCHAR* IniFilename, const FString& ContentsToSave, int32& SavedCount);
};
