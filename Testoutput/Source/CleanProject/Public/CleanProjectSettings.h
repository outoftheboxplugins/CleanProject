// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "CleanProjectSettings.generated.h"

/**
 * Holds the configurable settings for the Clean Project
 */

UCLASS(config = Editor)
class UCleanProjectSettings : public UObject
{
	GENERATED_BODY()

public:
	UCleanProjectSettings();

	// Outputes the blacklist result to a intermediate file instead of creating them directly.
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bUseSmartBlackList = true;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> PlatformsPaths;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> BlacklistFiles;

	UPROPERTY(EditAnywhere, config, Category = "Report")
	TArray<FString> ReportHiddenColumns;

};
