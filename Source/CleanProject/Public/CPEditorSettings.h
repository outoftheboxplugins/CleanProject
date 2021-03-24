// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "CPEditorSettings.generated.h"

/**
 * Holds the configurable settings for the Clean Project
 */

UCLASS(config = Editor)
class UCPEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UCPEditorSettings();

	// Outputs the blacklist result to a intermediate file instead of creating them directly.
	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bUseSmartBlackList = true;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> PlatformsPaths;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> BlacklistFiles;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bShouldAppendDefault = false;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bShouldSkipBlacklistDialog = false;

	UPROPERTY(EditAnywhere, config, Category = "Report")
	TArray<FString> ReportHiddenColumns;
};
