// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPSettings.h"

#include "CPOperationsSubsystem.h"

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
	TSet<FAssetData> Result;

	for (const FDirectoryPath& DirectoryPath : CoreFolders)
	{
		TArray<FAssetData> AssetData = UCPOperationsSubsystem::Get()->GetAssetsInPaths(DirectoryPath.Path);
		Result.Append(AssetData);
	}

	Algo::Transform(CoreAssets, Result,
		[](const FSoftObjectPath& Path)
		{
			const FString& AssetPath = Path.GetAssetPathString();
			return UEditorAssetLibrary::FindAssetData(AssetPath);
		});
	return Result;
}

TSet<FAssetData> UCPSettings::GetAssetsExcludedFromPackage() const
{
	TSet<FAssetData> Result;

	for (const FDirectoryPath& DirectoryPath : FoldersExcludedFromPackage)
	{
		TArray<FAssetData> AssetData = UCPOperationsSubsystem::Get()->GetAssetsInPaths(DirectoryPath.Path);
		Result.Append(AssetData);
	}

	Algo::Transform(AssetsExcludedFromPackage, Result,
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
	const FString IniFile = FString(IniFilename);
	if (IniFile == GetDefaultConfigFilename())
	{
		OnSettingsChanged.Broadcast();
	}
}
