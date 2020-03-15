// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate/Public/Widgets/Views/SListView.h"

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

// Custom Report column
private:
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
// Internal state
private:
	TArray<FAssetData> ReportAssets;
};