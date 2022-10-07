// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPSettings.h"

#include "AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "ISettingsModule.h"

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
		WhitelistedAssets.Emplace(FSoftObjectPath(OldWhitelistPath));
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
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
		WhitelistedAssets.Emplace(AssetPath);
	}

	SaveToDefaultConfig();
}

void UCPSettings::BlacklistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		FSoftObjectPath AssetPath = FSoftObjectPath(Asset.PackageName.ToString());
		BlacklistedAssets.Emplace(AssetPath);
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

void UCPSettings::IncreaseSpaceGained(int64 ExtraSpaceGained)
{
	SpaceGained += ExtraSpaceGained;
	SaveToDefaultConfig();
}

int64 UCPSettings::GetSpaceGained() const
{
	return SpaceGained;
}

void UCPSettings::SaveToDefaultConfig()
{
	SaveConfig(CPF_Config, *GetDefaultConfigFilename());
}

UCPSettings::UCPSettings()
{
	// TOSOLVE: check if the whitelisted asset path exists when loading the variable.
	// TOSOLVE: auto update paths when assets are moved.

	// TOSOLVE: cleanup or expose those.
	ReportHiddenColumns = {
		"Class",
		"CookRule",
		"Chunks",
		"Format",
		"sRGB",
		"TextureGroup",
		"Dimensions",
		"HasAlphaChannel",
		"AddressY",
		"AddressX",
		"Type",
		"ParentClass",
		"ModuleName",
		"ModuleRelativePath",
		"Dimensions",
		"HasAlphaChannel",
		"Format",
		"AddressX",
		"AddressY",
		"LODBias",
		"SRGB",
		"NeverStream",
		"CompressionSettings",
		"MipLoadOptions",
		"Filter",
		"MipLoadOptions",
		"LODGroup",
		"VirtualTexture",
		"VirtualTextureStreaming",
		"NeverStream",
		"PrimaryAssetType",
		"PrimaryAssetName",
		"DateModified",
		"BlueprintType",
		"NativeParentClass",
		"NumReplicatedProperties",
		"IsDataOnly",
		"NativeComponents",
		"BlueprintComponents",
		"Triangles",
		"Vertices",
		"UVChannels",
		"Materials",
		"ApproxSize",
		"CollisionPrims",
		"LODs",
		"MinLOD",
		"SectionsWithCollision",
		"DefaultCollision",
		"CollisionComplexity",
		"CompressionRatio",
		"Compression Ratio",
		"Compressed",
		"CompressedSize",
		"Compressed Size",
		"Compressed Size (KB)",
		"FrameRate",
		"NumFrames",
		"Interpolation",
		"RetargetSource",
		"AdditiveAnimType",
		"ImportFileFramerate",
		"ImportResampleFramerate",
		"bEnableRootMotion",
		"bUseNormalizedRootMotionScale",
		"SequenceLength",
		"RetargetSource",
	};
}
