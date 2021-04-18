// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"

class SCPAssetDialog : public SCompoundWidget
{
// Slate interface
public:
	SLATE_BEGIN_ARGS(SCPAssetDialog) { }
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);	

// Window commands
public:
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);
	void CloseAssetDialog();

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

private:
	TArray<FAssetData> GetAssetsForAction() const;
	TSharedRef<SWidget> CreateAssetPickerWidget();

// Internal state
private:
	TArray<FAssetData> ReportAssets;
	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate;

	FARFilter ReportAssetsFilter;
	FSetARFilterDelegate SetFilterDelegate;
};