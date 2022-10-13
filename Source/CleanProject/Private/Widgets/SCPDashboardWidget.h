// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SCompoundWidget.h"

using FCPAssetPtr = TSharedPtr<FAssetData>;

/**
 * Widget that represents an asset entry in the dashboard.
 */
class SCPDashboardAssetRow final : public SMultiColumnTableRow<TSharedPtr<FName>>
{
public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FCPAssetPtr InListItem);

private:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	FCPAssetPtr Item;
};

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

	FReply OnRunCleanupFast();
	FReply OnRunCleanupComplex();
	FReply OnRefreshUnused();
	FReply OnGoToDocumentation();
	FReply OnOpenSettings();

	bool ShouldReactToAssetChange(const FAssetData& AssetData) const;
	void RefreshUnusedAssets();

private:
	TSharedPtr<SListView<FCPAssetPtr>> InuseAssetsListView;
	TArray<FCPAssetPtr> InuseAssetsList;

	TSharedPtr<SListView<FCPAssetPtr>> UnusedAssetsListView;
	TArray<FCPAssetPtr> UnusedAssetsList;

	FDateTime LastRefreshTime;
	bool bIsIndexOutdated = true;
};
