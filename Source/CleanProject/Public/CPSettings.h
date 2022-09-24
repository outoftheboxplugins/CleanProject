// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "CPSettings.generated.h"

/**
 * Holds the configurable settings for the Clean Project plugin
 */
UCLASS(config = Editor, defaultconfig)
class UCPSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Shortcut to open the project settings windows focused to this config
	 */
	static void OpenSettings();
	/**
	 * List of assets we always consider actively referenced.
	 * Add assets (including their dependencies) you want to prevent our system from deleting here
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	TSet<FSoftObjectPath> WhitelistedAssets;
	/**
	 * Assets we should actively exclude from the final pak
	 * Add assets you want to prevent from getting added to your final game. Their dependencies might still get packaged if
	 * referenced by something else
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	TSet<FSoftObjectPath> BlacklistedAssets;
	/**
	 * Should we use treat the explicitly cooked/packaged maps as whitelisted?
	 * TODO: Should we handle the special Game/Maps folder ?
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	bool bWhitelistMapsToPackage = true;
	/**
	 * Convenience function to programatically add assets to the whitelisted set and saving config
	 */
	void WhitelistAssets(const TArray<FAssetData> Assets);
	/**
	 * Convenience function to programatically add assets to the blacklisted set and saving config
	 */
	void BlacklistAssets(const TArray<FAssetData> Assets);
	/**
	 * Increases the space gained metric and saves config
	 */
	void IncreaseSpaceGained(int64 ExtraSpaceGained);
	/**
	 * Returns the space gained by using CleanProject
	 */
	int64 GetSpaceGained() const
	{
		return SpaceGained;
	}

	/**
	 * Returns the asset path of the whitelisted assets as FName
	 */
	TSet<FName> GetWhitelistAssetsPaths() const;

	/**
	 * Old version for storing assets we want to whitelist
	 * TODO: Delete this when conversion is done
	 */
	UE_DEPRECATED(5.0, "WhitelistAssetsPaths has been removed as a way of storing references. Please use WhitelistedAssets")
	UPROPERTY(VisibleDefaultsOnly, config, Category = "Deprecated")
	TArray<FString> WhitelistAssetsPaths;

private:
	/**
	 * How many bytes we've saved so far by deleting assets with this system
	 */
	UPROPERTY(config)
	int64 SpaceGained = 0;

	// Begin UDeveloperSettings interface
	virtual void PostInitProperties() override;
	virtual FName GetContainerName() const override
	{
		return TEXT("Project");
	}
	virtual FName GetCategoryName() const override
	{
		return TEXT("Out-of-the-Box Plugins");
	}
	virtual FName GetSectionName() const override
	{
		return TEXT("Clean Project");
	}
	// End UDeveloperSettings interface

	/**
	 * Saves the current values to the .ini file inside root Config folder
	 */
	void SaveToDefaultConfig();

	// Backwards compatibility
	// ***************************************************************************************************************************
	// TODO: Review if we still want to use the hidden report columns
	UCPSettings();

public:
	// Columns to be hidden in the final report
	UPROPERTY(EditAnywhere, config, Category = "Report")
	TArray<FString> ReportHiddenColumns;
};
