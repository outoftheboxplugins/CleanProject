// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Editor/EditorEngine.h"

namespace CPMenuExtensions
{
	TSharedRef<SDockTab> SpawnMenuTab(const FSpawnTabArgs& Args);

	TSharedRef<FExtender> CreateContentBrowserAssetsExtender(const TArray<FAssetData>& SelectedAssets);
	void CreateContentBrowserAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

	TSharedRef<FExtender> CreateContentBrowserFoldersExtender(const TArray<FString>& SelectedFolders);
	void CreateContentBrowserFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders);
}