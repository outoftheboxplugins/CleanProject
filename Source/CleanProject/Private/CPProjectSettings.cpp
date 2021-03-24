// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CPProjectSettings.h"

UCPProjectSettings::UCPProjectSettings()
{ }

void UCPProjectSettings::WhitelistAsset(const FAssetData& Asset)
{
	WhitelistAsset(Asset.ObjectPath);
}

void UCPProjectSettings::WhitelistAsset(const FName& AssetPath)
{
	WhitelistAssetsPaths.Add(AssetPath);
}

void UCPProjectSettings::WhitelistAssetes(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		WhitelistAsset(Asset);
	}
}

void UCPProjectSettings::WhitelistAssetes(const TArray<FName> AssetPaths)
{
	for (const FName& AssetPath : AssetPaths)
	{
		WhitelistAsset(AssetPath);
	}
}
