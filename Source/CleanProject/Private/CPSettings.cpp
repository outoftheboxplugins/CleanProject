// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPSettings.h"

UCPSettings::UCPSettings()
{
	// TOSOLVE: check if the whitelisted asset path exists when loading the variable.
	// TOSOLVE: auto update paths when assets are moved.
	
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

	// TOSOLVE: cleanup or expose those.
    ReportHiddenColumns =
    {
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

void UCPSettings::WhitelistAssets(const TArray<FAssetData> Assets)
{
	for (const FAssetData& Asset : Assets)
	{
		WhitelistAssetsPaths.Add(Asset.PackageName.ToString());
	}

	SaveToDefaultConfig();
	OnAnyPropertyChanged.Broadcast();
}

TArray<FName> UCPSettings::GetWhitelistAssetsPaths() const
{
	TArray<FName> WhitelistPaths;
	for (const FString& Path : WhitelistAssetsPaths)
	{
		WhitelistPaths.Add(FName(Path));
	}

	return WhitelistPaths;
}

void UCPSettings::IncreaseSpaceGained(int64 ExtraSpaceGained)
{
	SpaceGained += ExtraSpaceGained;

	SaveToDefaultConfig();

	OnAnyPropertyChanged.Broadcast();
}

void UCPSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SaveToDefaultConfig();
	
	OnAnyPropertyChanged.Broadcast();
}

void UCPSettings::SaveToDefaultConfig()
{
	SaveConfig();

	GConfig->SetArray(TEXT("/Script/CleanProject.CPSettings"), TEXT("WhitelistAssetsPaths"), WhitelistAssetsPaths, GetDefaultConfigFilename());
	GConfig->SetInt(TEXT("/Script/CleanProject.CPSettings"), TEXT("SpaceGained"), SpaceGained, GetDefaultConfigFilename());
}
