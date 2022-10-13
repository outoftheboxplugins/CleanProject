// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "ARFilter.h"
#include "ContentBrowserDelegates.h"
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCPUnusedAssetsReport : public SCompoundWidget
{
public:
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);

	SLATE_BEGIN_ARGS(SCPUnusedAssetsReport)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);
	void CloseAssetDialog();

private:
	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets);

	TSharedRef<SWidget> CreateAssetPickerWidget();

	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate;
	FSetARFilterDelegate SetFilterDelegate;

	TArray<FAssetData> ReportAssets;
	FARFilter ReportAssetsFilter;

	void OnAssetDoubleClicked(const FAssetData& AssetData);
	FReply OnReferenceViewerClicked();
	FReply OnAuditClicked();
	FReply OnDeleteClicked();
	FReply OnWhitelistClicked();
	FReply OnBlacklistClicked();
	FReply OnCancelClicked();

	void ReferenceViewerAssets(const TArray<FAssetData> AssetsToViewReferences);
	void AuditAssets(const TArray<FAssetData> AssetsToAudit);
	void DeleteAssets(const TArray<FAssetData> AssetsToDelete);
	void WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist);
	void BlackListAssets(const TArray<FAssetData> AssetsToBlacklist);

	void RemoveFromList(const TArray<FAssetData> AssetsToRemove);

	TArray<FAssetData> GetAssetsForAction() const;
	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList) const;
	int64 GetAssetDiskSize(const FAssetData& Asset) const;
};
