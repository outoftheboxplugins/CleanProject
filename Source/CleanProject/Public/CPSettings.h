// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "CPSettings.generated.h"

/**
 * Holds the configurable settings for the Clean Project
 */

UCLASS(config = Editor)
class UCPSettings : public UObject
{
	GENERATED_BODY()

public:
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

private:
	// Assets inside the whitelist are always considered referenced.
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	TArray<FString> WhitelistAssetsPaths;

public:
	// Columns to be hidden in the final report
	UPROPERTY(EditAnywhere, config, Category = "Report")
	TArray<FString> ReportHiddenColumns;

private:
	UPROPERTY(VisibleAnywhere, config, Category = "Report")
	int64 SpaceGained = 0;
};
