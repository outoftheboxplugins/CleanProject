// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

namespace CPMenuExtensions
{
TSharedRef<FExtender> CreateContentBrowserAssetsExtender(const TArray<FAssetData>& SelectedAssets);
void CreateContentBrowserAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

TSharedRef<FExtender> CreateContentBrowserFoldersExtender(const TArray<FString>& SelectedFolders);
void CreateContentBrowserFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders);
}	 // namespace CPMenuExtensions
