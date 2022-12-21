// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPSettings.h"

#include <EditorAssetLibrary.h>
#include <ISettingsModule.h>

#include "CPHelpers.h"

namespace
{
	TSet<FAssetData> GetAssetsFromObjectsAndPaths(const TArray<FSoftObjectPath>& InObjects, const TArray<FDirectoryPath>& InPaths)
	{
		TSet<FAssetData> Result;

		for (const FDirectoryPath& DirectoryPath : InPaths)
		{
			TArray<FAssetData> AssetData = CPHelpers::GetAssetsInPaths(DirectoryPath.Path);
			Result.Append(AssetData);
		}

		Algo::Transform(
			InObjects,
			Result,
			[](const FSoftObjectPath& ObjectPath)
			{
				const FString& AssetPath = ObjectPath.GetAssetPathString();
				return UEditorAssetLibrary::FindAssetData(AssetPath);
			}
		);
		return Result;
	}
} // namespace

void UCPSettings::OpenSettings()
{
	const UCPSettings* Settings = GetDefault<UCPSettings>();

	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.ShowViewer(Settings->GetContainerName(), Settings->GetCategoryName(), Settings->GetSectionName());
}

void UCPSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Backwards compatibility
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	for (const FString& OldCorePath : WhitelistAssetsPaths)
	{
		CoreAssets.Add(FSoftObjectPath(OldCorePath));
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	FCoreDelegates::PreSaveConfigFileDelegate.AddUObject(this, &UCPSettings::OnAnyConfigSaved);
}

FName UCPSettings::GetContainerName() const
{
	return TEXT("Project");
}

FName UCPSettings::GetCategoryName() const
{
	return TEXT("Out-of-the-Box Plugins");
}

FName UCPSettings::GetSectionName() const
{
	return TEXT("Clean Project");
}

#if WITH_EDITOR
FText UCPSettings::GetSectionText() const
{
	const FName DisplaySectionName = GetSectionName();
	return FText::FromName(DisplaySectionName);
}
#endif

void UCPSettings::MarkAssetsAsCore(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		FSoftObjectPath AssetPath = FSoftObjectPath(Asset.PackageName.ToString());
		CoreAssets.Add(AssetPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::MarkPathsAsCore(const TArray<FString> Paths)
{
	for (const FString& Path : Paths)
	{
		FDirectoryPath DirectoryPath = {Path};
		CoreFolders.Add(DirectoryPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::ExcludeAssetsFromPackage(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		FSoftObjectPath AssetPath = FSoftObjectPath(Asset.PackageName.ToString());
		AssetsExcludedFromPackage.Add(AssetPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::ExcludePathsFromPackage(const TArray<FString> Paths)
{
	for (const FString& Path : Paths)
	{
		FDirectoryPath DirectoryPath = {Path};
		FoldersExcludedFromPackage.Add(DirectoryPath);
	}

	SaveToDefaultConfig();
}

TSet<FAssetData> UCPSettings::GetCoreAssets() const
{
	return GetAssetsFromObjectsAndPaths(CoreAssets, CoreFolders);
}

TSet<FAssetData> UCPSettings::GetAssetsExcludedFromPackage() const
{
	return GetAssetsFromObjectsAndPaths(AssetsExcludedFromPackage, FoldersExcludedFromPackage);
}

void UCPSettings::SaveToDefaultConfig()
{
	SaveConfig(CPF_Config, *GetDefaultConfigFilename());
}

void UCPSettings::OnAnyConfigSaved(const TCHAR* IniFilename, const FString& ContentsToSave, int32& SavedCount)
{
	const FString IniFile = FString(IniFilename);
	if (IniFile == GetDefaultConfigFilename())
	{
		OnSettingsChanged.Broadcast();
	}
}