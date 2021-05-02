// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"

/**
 * Menu Widget containing a UI interface for the developer to interact with the Clean Project.
 */

using FAssetDataPtr = TSharedPtr<FName>;

enum class ECPAssetDependencyType : uint8
{
	None,
	MapAssets,
	WhitelistAssets,
	AnyAssets,
};

class SCPMenuWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCPMenuWidget) { }
	SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
	~SCPMenuWidget();

private:
	TSharedRef<SWidget> CreateInfoWidget(FText Title, TAttribute<FText> MetricValueAttribute);

	FText GetColumnNameByType(ECPAssetDependencyType AssetDependencyType) const;
	FText GetMapAssetsColumnName() const { return GetColumnNameByType(ECPAssetDependencyType::MapAssets); }
	FText GetWhitelistAssetsColumnName() const { return GetColumnNameByType(ECPAssetDependencyType::WhitelistAssets); }

	void OnGetChildren(FAssetDataPtr InItem, TArray<FAssetDataPtr>& OutChildren);

// Resizing
private:
	void OnInfoSlotResized(float newSize) { UniformInfoSlotSize = newSize; }
	float GetInfoSlotSizeLeft() const { return UniformInfoSlotSize; }
	float GetInfoSlotSizeRight() const { return 1.0f - UniformInfoSlotSize; }

// Refrshing
private:
	FTimerHandle RefreshTimerHandle;
	int64 GetUnusedAssetsCount() const;
	void RefreshUnusedAssets();

// Buttons
private:
	FReply OnRunCleanupNow();
	FReply OnRefreshUnushed();
	FReply OnGoToDocumentation();
	FReply OnOpenSettings();

private:
	TSharedPtr<STreeView<FAssetDataPtr>> DependenciesTreeView;
	TArray<FAssetDataPtr> TopLevelDependencies;

	TSharedPtr<SListView<FAssetDataPtr>> MapAssetsListView;
	TArray<FAssetDataPtr> MapAssets;

	TSharedPtr<SListView<FAssetDataPtr>> WhitelistAssetsListView;
	TArray<FAssetDataPtr> WhitelistAssets;

	int64 UnusedAssetsCount = 1024;
	float UniformInfoSlotSize = 0.5f;
};
