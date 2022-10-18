// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <Engine/DeveloperSettings.h>

#include "CPSettings.generated.h"

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
	 * @brief List of assets we always consider actively referenced. Add assets (including their dependencies) you want to prevent
	 * our system from deleting here
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	TSet<FSoftObjectPath> WhitelistedAssets;
	/**
	 * @brief Old version for storing assets we want to whitelist
	 */
	UE_DEPRECATED(5.0, "WhitelistAssetsPaths has been removed as a way of storing references. Please use WhitelistedAssets")
	UPROPERTY(VisibleDefaultsOnly, config, Category = "Deprecated")
	TArray<FString> WhitelistAssetsPaths;
	/**
	 * @brief Assets we should actively exclude from the final pak
	 * Add assets you want to prevent from getting added to your final game. Their dependencies might still get packaged if
	 * referenced by something else
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	TSet<FSoftObjectPath> BlacklistedAssets;
	/**
	 * @brief  Should the Clean Project Dashboard automatically refresh itself every time an asset is updated. (Unless time since
	 * last refresh is lower than AutoRefreshInterval)
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	bool bAutoRefreshDashboard = true;
	/**
	 * @brief How much time we should wait in between Clean Project Dashboard refresh requests
	 * @note measured in seconds
	 * @note use -1 to refresh every time
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	float AutoRefreshInterval = 30.0f;
	/**
	 * @brief Convenience function to programatically add assets to the whitelisted set and saving config
	 */
	void WhitelistAssets(const TArray<FAssetData> Assets);
	/**
	 * @brief Convenience function to programatically add assets to the blacklisted set and saving config
	 */
	void BlacklistAssets(const TArray<FAssetData> Assets);
	/**
	 * @brief Returns the asset path of the whitelisted assets as FName
	 */
	TSet<FAssetData> GetWhitelistAssetsPaths() const;

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
