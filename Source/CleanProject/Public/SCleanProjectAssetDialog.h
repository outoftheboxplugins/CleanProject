// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate/Public/Widgets/Views/SListView.h"

class SAssetView;

class SCleanProjectAssetDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCleanProjectAssetDialog) {}
	SLATE_END_ARGS()

// Interface
public:
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);
	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);
	void CloseDialog();

// Slate Delegates
private:
	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets);
	void OnRequestOpenAsset(const FAssetData& AssetData) const;

// Custom Report column
private:
	int64 GetAssetDiskSize(const FAssetData& Asset) const;
	FString GetDiskSizeData(FAssetData& AssetData, FName ColumnName) const;
	FText GetDiskSizeDisplayText(FAssetData& AssetData, FName ColumnName) const;

// Buttons Actions
private:
	FReply OnDeleteClicked();
	FReply OnAuditClicked();
	FReply OnBlacklistClicked();
	FReply OnCancelClicked();

// Functionality
private:
	void DeleteAssets(const TArray<FAssetData> AssetsToDelete);
	void AuditAssets(const TArray<FAssetData> AssetsToAudit);
	void BlackListAssets(const TArray<FAssetData> AssetsToBlacklist);
	void WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist);
	void RemoveFromList(const TArray<FAssetData> AssetsToRemove);

// Internal state
private:
	TArray<FAssetData> ReportAssets;

	FSetARFilterDelegate SetFilterDelegate;
	FARFilter ReportAssetsFilter;
};