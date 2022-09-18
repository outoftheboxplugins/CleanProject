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
	 * Should we use treat the explicitly cooked/packaged maps as whitelisted?
	 * TODO: Should we handle the special Game/Maps folder ?
	 */
	UPROPERTY(EditDefaultsOnly, config, Category = "Whitelist")
	bool bWhitelistMapsToPackage = true;

	UE_DEPRECATED(5.0, "WhitelistAssetsPaths has been removed as a way of storing references. Please use UnusedAssetsList")
	UPROPERTY(VisibleDefaultsOnly, config, Category = "Deprecated")
	TArray<FString> WhitelistAssetsPaths;

private:
	/**
	 * How many bytes we've saved so far by deleting assets with this system
	 */
	UPROPERTY(config)
	int64 SpaceGained = 0;

	virtual void PostInitProperties() override;

	virtual FName GetContainerName() const override		{ return TEXT("Project"); }
	virtual FName GetCategoryName() const override		{ return TEXT("Out-of-the-Box Plugins"); }
	virtual FName GetSectionName() const override		{ return TEXT("Clean Project"); }






	// Backwards compatibility ***************************************************************************************************************************
	UCPSettings();

public:
	void WhitelistAssets(const TArray<FAssetData> Assets);
	TArray<FName> GetWhitelistAssetsPaths() const;

	void IncreaseSpaceGained(int64 ExtraSpaceGained);
	int64 GetSpaceGained() const { return SpaceGained; }

private:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	void SaveToDefaultConfig();
public:
	
	FSimpleMulticastDelegate OnAnyPropertyChanged;

public:
	// Outputs the blacklist result to a intermediate file instead of creating them directly
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bSaveToTempFile = true;

	// Possible platforms for the blacklist
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> PlatformsPaths;

	// Possible files for the blacklist
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> BlacklistFiles;

	// Prefer to append to the existing list instead of the overwriting it
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bShouldAppendDefault = false;

	// Prefer to skip the blacklist dialog and go with the default option (export for all platforms & append to existing)
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bShouldSkipBlacklistDialog = false;

	// Should we always check the dependencies of the whitelisted assets when scanning for references?
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	bool bCheckWhitelistReferences = true;

	// When performing a project-wide clean, should all the maps be included in the reference check?
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	bool bCheckAllMapsReferences = true;
public:
	// Columns to be hidden in the final report
	UPROPERTY(EditAnywhere, config, Category = "Report")
	TArray<FString> ReportHiddenColumns;
};
