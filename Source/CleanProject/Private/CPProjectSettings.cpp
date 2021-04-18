// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#include "CPProjectSettings.h"

void UCPProjectSettings::WhitelistAsset(const FAssetData& Asset)
{
	WhitelistAsset(Asset.ObjectPath);
}

void UCPProjectSettings::WhitelistAsset(const FName& AssetPath)
{
	WhitelistAssetsPaths.Add(AssetPath);
}

void UCPProjectSettings::WhitelistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		WhitelistAsset(Asset);
	}
}

void UCPProjectSettings::WhitelistAssets(const TArray<FName> AssetPaths)
{
	for (const FName& AssetPath : AssetPaths)
	{
		WhitelistAsset(AssetPath);
	}
}
