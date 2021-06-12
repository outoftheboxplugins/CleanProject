// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPSettings.h"

UCPSettings::UCPSettings()
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

void UCPSettings::WhitelistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		WhitelistAssetsPaths.Add(Asset.PackageName);
	}

	SaveConfig();

	OnAnyPropertyChanged.Broadcast();
}

void UCPSettings::IncreaseSpaceGained(int64 ExtraSpaceGained)
{
	SpaceGained += ExtraSpaceGained;
	SaveConfig();

	OnAnyPropertyChanged.Broadcast();
}

void UCPSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnAnyPropertyChanged.Broadcast();
}
