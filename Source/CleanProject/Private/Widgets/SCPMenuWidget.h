// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Menu Widget containing a UI interface for the developer to interact with the Clean Project.
 */
class SCPMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCPMenuWidget)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void OnFilesLoaded();
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
	int64 GetUnusedAssetsCount() const;

	void RefreshUnusedAssets();

private:
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> InuseAssetsListView;
	TArray<TSharedPtr<FAssetData>> InuseAssetsList;

	TSharedPtr<SListView<TSharedPtr<FAssetData>>> UnusedAssetsListView;
	TArray<TSharedPtr<FAssetData>> UnusedAssetsList;

	FDateTime LastRefreshTime;
	bool bIsIndexOutdated = true;
};
