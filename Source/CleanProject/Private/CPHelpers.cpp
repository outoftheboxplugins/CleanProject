// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPHelpers.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetViewUtils.h"
#include "Engine/AssetManager.h"

TSet<FAssetData> CPHelpers::GetAllGameAssets(TOptional<FTopLevelAssetPath> ClassFilter)
{
	TArray<FAssetData> AllAssetData;

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	if (ClassFilter.IsSet())
	{
		Filter.ClassPaths.Add(ClassFilter.GetValue());
	}

	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AllAssetData);
	return TSet(AllAssetData);
}

FAssetData CPHelpers::GetDefaultGameObject(const FName& PropertyName)
{
	FConfigFile PlatformEngineIni;
	FConfigCacheIni::LoadLocalIniFile(PlatformEngineIni, TEXT("Engine"), true);

	FConfigSection* MapSettingsSection = PlatformEngineIni.Find(TEXT("/Script/EngineSettings.GameMapsSettings"));
	if (MapSettingsSection == nullptr)
	{
		return {};
	}

	const FConfigValue* PairString = MapSettingsSection->Find(PropertyName);
	const FString ObjectPath = PairString ? PairString->GetValue() : TEXT("");
	const FSoftClassPath Test = FSoftClassPath(ObjectPath);

	FAssetData Result;
	UAssetManager& AssetManager = UAssetManager::Get();
	AssetManager.GetAssetDataForPath(ObjectPath, Result);

	return Result;
}

TArray<FAssetData> CPHelpers::GetAssetsInPaths(TArray<FString> FolderPaths)
{
	TArray<FAssetData> AllAssetData;
	AssetViewUtils::GetAssetsInPaths(FolderPaths, AllAssetData);

	return AllAssetData;
}

TArray<FAssetData> CPHelpers::GetAssetsInPaths(FString FolderPath)
{
	const TArray<FString> FolderPaths = {FolderPath};
	return GetAssetsInPaths(FolderPaths);
}
