// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"

#include "CPOperations.h"
#include "Widgets/SWindow.h"
#include "Editor/EditorEngine.h"
#include "Templates/SharedPointer.h"
#include "Widgets/Views/STreeView.h"
/**
 * Menu Widget containing a UI interface for the developer to interact with the Clean Project.
 */

enum class ECPAssetDependencyType : uint8
{
	None,
	MapAssets,
	WhitelistAssets,
	AnyAssets,
};

class SCPAssetDependencyRow final : public SMultiColumnTableRow<FAssetDataPtr>
{
public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FAssetDataPtr InListItem, ECPAssetDependencyType InAssetDependencyType);

private:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	FSlateColor GetTextColor() const;

private:
	FAssetDataPtr Item;
	ECPAssetDependencyType AssetDependencyType = ECPAssetDependencyType::None;
};

class SCPMenuWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCPMenuWidget) { }
	SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
	
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

// Refreshing
private:
	void OnFilesLoaded();
	void OnAssetAdded(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
	void OnAssetRenamed(const FAssetData& AssetData, const FString& Name);
	void OnAssetUpdated(const FAssetData& AssetData);
	
	int64 GetUnusedAssetsCount() const;
	void RefreshUnusedAssets();

	bool IsGameAsset(const FAssetData& AssetData) const;

// Buttons
private:
	FReply OnRunCleanupNow();
	FReply OnRefreshUnused();
	FReply OnGoToDocumentation();
	FReply OnOpenSettings();

private:
	TSharedPtr<STreeView<FAssetDataPtr>> DependenciesTreeView;
	CPOperations::FTreeAssetDependency AssetsDependencies;

	TSharedPtr<SListView<FAssetDataPtr>> MapAssetsListView;
	TArray<FAssetDataPtr> MapAssets;

	TSharedPtr<SListView<FAssetDataPtr>> WhitelistAssetsListView;
	TArray<FAssetDataPtr> WhitelistAssets;

	int64 UnusedAssetsCount = 1024;
	float UniformInfoSlotSize = 0.5f;
};
