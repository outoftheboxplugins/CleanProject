// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

namespace CPMenuExtensions
{
	TSharedPtr<FExtender> CreateMenuExtender();
	void AddMenuExtension(FMenuBuilder& MenuBuilder);

	TSharedRef<FExtender> CreateContentBrowserExtender(const TArray<FAssetData>& SelectedAssets);
	void CreateContentBrowserEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
}