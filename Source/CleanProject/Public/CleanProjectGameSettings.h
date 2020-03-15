// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
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

	// Assets inside the whitelist are always considered referenced.
	UPROPERTY(EditAnywhere, config, Category = "Whitelist")
	TArray<FName> WhiteListAssetsPaths;
};
