// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <Widgets/SCompoundWidget.h>

class SAssetView;

/**
 * NomadTab containing an overview of the current project state.
 * Displays: list whitelisted assets and list of unused assets
 * Shortcut buttons to different actions: running cleanups, refreshing, opening settings
 */
class SCPDashboardWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCPDashboardWidget)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void OnInitialScanComplete();
	void OnAssetAdded(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
	void OnAssetRenamed(const FAssetData& AssetData, const FString& Name);
	void OnAssetUpdated(const FAssetData& AssetData);
	void OnSettingsChanged();

	FReply OnRunCleanupFast();
	FReply OnRunCleanupComplex();
	FReply OnRefreshUnused();
	FReply OnGoToDocumentation();
	FReply OnOpenSettings();

	bool ShouldReactToAssetChange(const FAssetData& AssetData) const;
	void RefreshUnusedAssets();

	bool FilterInuseAsset(const FAssetData& AssetData) const;
	bool FilterUnusedAsset(const FAssetData& AssetData) const;

private:
	TArray<FAssetData> InuseAssets;
	TSharedPtr<SAssetView> InuseAssetView;

	TArray<FAssetData> UnusedAssets;
	TSharedPtr<SAssetView> UnusedAssetView;

	FDateTime LastRefreshTime;
	bool bIsIndexOutdated = true;
};
