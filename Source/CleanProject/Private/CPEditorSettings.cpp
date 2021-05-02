// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#include "CPEditorSettings.h"

UCPEditorSettings::UCPEditorSettings()
{
    PlatformsPaths = 
    { 
        "WindowsNoEditor",
        "Android",
        "IOS",
        "Mac",
        "Linux"
    };

    BlacklistFiles =
    { 
        "PakBlacklist-Debug.txt",
        "PakBlacklist-Development.txt",
        "PakBlacklist-Test.txt",
        "PakBlacklist-Shipping.txt",
    };

    ReportHiddenColumns =
    {
		"Class",
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
		"CompressionSettings",
		"Filter",
		"MipLoadOptions",
		"LODGroup",
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
		"RegargetSource",
		"AdditiveAnimType",
		"ImportFileFramerate",
		"ImportResampleFramerate",
		"bEnableRootMotion",
		"bUseNormalizedRootMotionScale",
		"SequenceLength",
		"RetargetSource",
    };
}

void UCPEditorSettings::WhitelistAsset(const FAssetData& Asset)
{
	WhitelistAsset(Asset.ObjectPath);
}

void UCPEditorSettings::WhitelistAsset(const FName& AssetPath)
{
	WhitelistAssetsPaths.Add(AssetPath);
	SaveConfig();
}

void UCPEditorSettings::WhitelistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		WhitelistAsset(Asset);
	}
}

void UCPEditorSettings::WhitelistAssets(const TArray<FName> AssetPaths)
{
	for (const FName& AssetPath : AssetPaths)
	{
		WhitelistAsset(AssetPath);
	}
}

void UCPEditorSettings::IncreaseSpaceGained(int64 ExtraSpaceGained)
{
	SpaceGained += ExtraSpaceGained;
	SaveConfig();
}