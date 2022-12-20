// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <Widgets/SCompoundWidget.h>

class SAssetView;

/**
 * @brief Clean Project Dashboard containing an overview of the current project state and quick action buttons.
 */
class SCPDashboardWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCPDashboardWidget) {}
	SLATE_END_ARGS()

	/**
	 * @brief Creates the slate widget that represents the Clean Project Dashboard Tab
	 * @param InArgs A set of slate arguments, defined above.
	 */
	void Construct(const FArguments& InArgs);

private:
	// Begin SCompoundWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End SCompoundWidget interface
	/**
	 * @brief Callback executed after the initial scan of the assets is completed (or on tab spawn if scan was already completed)
	 */
	void OnInitialScanComplete();
	/**
	 * @brief Callback executed when a new asset is added
	 * @param AssetData Added asset
	 */
	void OnAssetAdded(const FAssetData& AssetData);
	/**
	 * @brief Callback executed when an existing asset is deleted
	 * @param AssetData Deleted asset
	 */
	void OnAssetDeleted(const FAssetData& AssetData);
	/**
	 * @brief Callback executed when an existing asset is renamed
	 * @param AssetData Renamed asset
	 * @param Name New asset name
	 */
	void OnAssetRenamed(const FAssetData& AssetData, const FString& Name);
	/**
	 * @brief Callback executed when an existing asset is updated
	 * @param AssetData Updated asset
	 */
	void OnAssetUpdated(const FAssetData& AssetData);
	/**
	 * @brief Callback executed when the Clean Project settings are changed
	 */
	void OnSettingsChanged();
	/**
	 * @brief Callback executed when a FastCleanup button is pressed
	 * @return if the operation was handled or not
	 */
	FReply OnRunCleanupFast();
	/**
	 * @brief Callback executed when ComplexCleanup button is pressed
	 * @return if the operation was handled or not
	 */
	FReply OnRunCleanupComplex();
	/**
	 * @brief Callback executed when Refresh button is pressed
	 * @return if the operation was handled or not
	 */
	FReply OnRefreshUnused();
	/**
	 * @brief Callback executed when Documentation button is pressed
	 * @return if the operation was handled or not
	 */
	FReply OnGoToDocumentation();
	/**
	 * @brief Callback executed when Settings button is pressed
	 * @return if the operation was handled or not
	 */
	FReply OnOpenSettings();

	/**
	 * @brief Checks if asset change should trigger a refresh. Takes into account:
	 * 1. AutoRefresh setting - developers can opt out of automatic refreshes if they prefer to trigger them manually
	 * 2. Outdated index -  we only update if a recent change has happened
	 * 3. Refresh interval - to prevent batch operations from triggering multiple refreshes developers can set a maximum rate
	 * @return true if we should refresh the list, false otherwise
	 */
	bool ShouldUpdateIndex() const;
	/**
	 * @brief Refreshes the cached state of the InUse & Unused assets
	 */
	void RefreshUnusedAssets();
	/**
	 * @brief Determine if a certain Asset should be displayed inside the core assets category
	 * @param AssetData Asset we are evaluating
	 * @return true to hide the asset, false to show the asset
	 */
	bool FilterCoreAssets(const FAssetData& AssetData) const;
	/**
	 * @brief Determine if a certain Asset should be displayed inside the Unused assets category
	 * @param AssetData Asset we are evaluating
	 * @return true to hide the asset, false to show the asset
	 */
	bool FilterUnusedAsset(const FAssetData& AssetData) const;
	/**
	 * @brief Cached core assets list at the time of the last refresh we performed
	 */
	TArray<FAssetData> CachedCoreAssets;
	/**
	 * @brief Slate widget displaying the cached core assets of the last refresh we performed
	 */
	TSharedPtr<SAssetView> CoreAssetsView;
	/**
	 * @brief Cached unused assets list at the time of the last refresh we performed
	 */
	TArray<FAssetData> CachedUnusedAssets;
	/**
	 * @brief Slate widget displaying the cached unused assets of the last refresh we performed
	 */
	TSharedPtr<SAssetView> UnusedAssetView;
	/**
	 * @brief Records the time of our last refresh performed
	 */
	FDateTime LastRefreshTime;
	/**
	 * @brief Records if a significant change happened since the last refresh
	 */
	bool bIsIndexDirty = true;
};
