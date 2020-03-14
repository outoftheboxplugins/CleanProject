// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectSettings.h"

UCleanProjectSettings::UCleanProjectSettings()
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
    };
}

