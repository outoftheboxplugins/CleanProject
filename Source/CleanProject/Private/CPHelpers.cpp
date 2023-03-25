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

TArray<FAssetData> CPHelpers::GetAssetsInIniFiles()
{
	TArray<FAssetData> Result;

	TArray<FString> IniFiles;
	GConfig->GetConfigFilenames(IniFiles);
	IniFiles.RemoveAll(
		[](const FString& FileName)
		{
			return !GConfig->IsKnownConfigName(FName(*FileName));
		}
	);

	for (const FString& ConfigFilename : IniFiles)
	{
		const FConfigFile* ConfigFile = GConfig->FindConfigFile(ConfigFilename);
		for (auto& ConfigSection : *ConfigFile)
		{
			for (auto& ConfigValue : ConfigSection.Value)
			{
				const FString Entry = ConfigValue.Value.GetValue();
				if (const UObject* ResolvedObject = FSoftObjectPath(Entry).TryLoad())
				{
					FAssetData AssetData = FAssetData(ResolvedObject);
					Result.Add(AssetData);
				}
			}
		}
	}

	return Result;
}
