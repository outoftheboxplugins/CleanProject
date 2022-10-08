// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "AssetData.h"
#include "CoreMinimal.h"

namespace CPOperations
{
int64 GetAssetDiskSize(const FAssetData& Asset);
int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList);
};	  // namespace CPOperations
