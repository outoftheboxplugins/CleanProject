// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

#include "CoreMinimal.h"
#include "CPOperations.h"

#include "Templates/SharedPointer.h"
#include "Widgets/Views/STreeView.h"
/**
 * Menu Widget containing a UI interface for the developer to interact with the Clean Project.
 */

class SCPMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCPMenuWidget) { }
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
private:
	TSharedRef<SWidget> CreateInfoWidget(FText Title, TAttribute<FText> MetricValueAttribute);

	void OnGetChildren(FAssetDataPtr InItem, TArray<FAssetDataPtr>& OutChildren);

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
	TSharedPtr<STreeView<FAssetDataPtr>> InuseAssetsTreeView;
	//TODO: What the fuck is this type?
	CPOperations::FTreeAssetDependency InuseAssetsDependencies;

	TSharedPtr<SListView<FAssetDataPtr>> UnusedAssetsListView;
	TArray<FAssetDataPtr> UnusedAssetsList;

	int64 UnusedAssetsCount = 0;
};
