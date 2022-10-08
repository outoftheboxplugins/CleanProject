// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "ARFilter.h"
#include "ContentBrowserDelegates.h"
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCPAssetDialog : public SCompoundWidget
{
public:
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);

	SLATE_BEGIN_ARGS(SCPAssetDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);
	void CloseAssetDialog();

private:
	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets);
	static void OnRequestOpenAsset(const FAssetData& AssetData);

	TArray<FAssetData> GetAssetsForAction() const;
	TSharedRef<SWidget> CreateAssetPickerWidget();
	
	int64 GetAssetDiskSize(const FAssetData& Asset);

	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList);

	FReply OnDeleteClicked();
	FReply OnMoreInfoClicked();
	FReply OnWhitelistClicked();
	FReply OnBlacklistClicked();
	FReply OnCancelClicked();

	void DeleteAssets(const TArray<FAssetData> AssetsToDelete);
	void MoreInfoAsset(const TArray<FAssetData> AssetsToGetInfo);
	void BlackListAssets(const TArray<FAssetData> AssetsToBlacklist);
	void WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist);
	void RemoveFromList(const TArray<FAssetData> AssetsToRemove);

	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate;
	FSetARFilterDelegate SetFilterDelegate;

	TArray<FAssetData> ReportAssets;
	FARFilter ReportAssetsFilter;
};
