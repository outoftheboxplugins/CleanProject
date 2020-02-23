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

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	bool bUseSmartBlackList = true;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	TArray<FString> PlatformsPaths;

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	FString DebugBlackList			= "PakBlacklist-Debug.txt";

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	FString DevelopmentBlackList	= "PakBlacklist-Development.txt";

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	FString TestBlackList			= "PakBlacklist-Test.txt";

	UPROPERTY(EditAnywhere, config, Category = "Blacklist")
	FString ShippingBlackList		= "PakBlacklist-Shipping.txt";

public:
	TArray<FString> GetAllBlackLists() const;
};
