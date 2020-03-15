// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AssetData.h"
#include "CleanProjectGameSettings.generated.h"

/**
 * Holds the configurable settings for the Clean Project
 */

UCLASS(config = Engine, defaultconfig)
class UCleanProjectGameSettings : public UObject
{
	GENERATED_BODY()

public:
	UCleanProjectGameSettings();

public:
	void WhitelistAsset(const FAssetData& Asset);
	void WhitelistAsset(const FName& AssetPath);

	void WhitelistAssetes(const TArray<FAssetData> Assets);
	void WhitelistAssetes(const TArray<FName> AssetPaths);

public:
	// Assets inside the whitelist are always considered referenced.
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	TArray<FName> WhitelistAssetsPaths;

	// Should we always check the dependencies of the whitelisted assets when scanning for references?
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	bool bCheckWhitelistReferences = true;

	// When performing a project-wide clean, should all the maps be included in the reference check?
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	bool bCheckAllMapsRefernece = true;
};

