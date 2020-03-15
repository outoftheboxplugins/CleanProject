// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectGameSettings.h"

UCleanProjectGameSettings::UCleanProjectGameSettings()
{ }

void UCleanProjectGameSettings::WhitelistAsset(const FAssetData& Asset)
{
	WhitelistAsset(Asset.ObjectPath);
}

void UCleanProjectGameSettings::WhitelistAsset(const FName& AssetPath)
{
	WhitelistAssetsPaths.Add(AssetPath);
}

void UCleanProjectGameSettings::WhitelistAssetes(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		WhitelistAsset(Asset);
	}
}

void UCleanProjectGameSettings::WhitelistAssetes(const TArray<FName> AssetPaths)
{
	for (const FName& AssetPath : AssetPaths)
	{
		WhitelistAsset(AssetPath);
	}
}
