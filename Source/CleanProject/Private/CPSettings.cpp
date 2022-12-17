// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPSettings.h"

#include <AssetData.h>
#include <EditorAssetLibrary.h>
#include <ISettingsModule.h>

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
	for (const FString& OldWhitelistPath : WhitelistAssetsPaths)
	{
		WhitelistedAssets.Add(FSoftObjectPath(OldWhitelistPath));
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

void UCPSettings::WhitelistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		FSoftObjectPath AssetPath = FSoftObjectPath(Asset.PackageName.ToString());
		WhitelistedAssets.Add(AssetPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::WhitelistPaths(const TArray<FString> Paths)
{
	for (const FString& Path : Paths)
	{
		FDirectoryPath DirectoryPath = {Path};
		WhitelistedFolders.Add(DirectoryPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::BlacklistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		FSoftObjectPath AssetPath = FSoftObjectPath(Asset.PackageName.ToString());
		BlacklistedAssets.Add(AssetPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::BlacklistPaths(const TArray<FString> Paths)
{
	for (const FString& Path : Paths)
	{
		FDirectoryPath DirectoryPath = {Path};
		BlacklistedFolders.Add(DirectoryPath);
	}

	SaveToDefaultConfig();
}

TSet<FAssetData> UCPSettings::GetWhitelistAssetsPaths() const
{
	TSet<FAssetData> Result;
	Algo::Transform(WhitelistedAssets, Result,
		[](const FSoftObjectPath& Path)
		{
			const FString& AssetPath = Path.GetAssetPathString();
			return UEditorAssetLibrary::FindAssetData(AssetPath);
		});
	return Result;
}

void UCPSettings::SaveToDefaultConfig()
{
	SaveConfig(CPF_Config, *GetDefaultConfigFilename());
}

void UCPSettings::OnAnyConfigSaved(const TCHAR* IniFilename, const FString& ContentsToSave, int32& SavedCount)
{
	FString IniFile = FString(IniFilename);
	if (IniFile == GetDefaultConfigFilename())
	{
		OnSettingsChanged.Broadcast();
	}
}
