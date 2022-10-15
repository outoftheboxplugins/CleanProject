// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <ContentBrowserDelegates.h>
#include <Widgets/SCompoundWidget.h>

/**
 * @brief Clean Project Report screen containing the reported assets after performing an operation
 */
class SCPUnusedAssetsReport : public SCompoundWidget
{
public:
	/**
	 * @brief Attempts to create a new report window which displays the AssetsToReport
	 * @param AssetsToReport Assets to display in the report
	 */
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);

	SLATE_BEGIN_ARGS(SCPUnusedAssetsReport)
	{
	}
	SLATE_END_ARGS()

	/**
	 * @brief Creates the slate widget that represents the Clean Project Report screen
	 * @param InArgs A set of slate arguments, defined above.
	 * @param AssetsToReport Assets this report should display
	 */
	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);

	// TODO: Research if this is even needed and if opening other windows automatically close this one, e.g.: reference viewer
	void CloseAssetDialog();

private:
	TSharedRef<SWidget> CreateAssetPickerWidget();
	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets);

	/**
	 * @brief Callback executed when an asset is double clicked
	 * @param AssetData Clicked asset
	 */
	void OnAssetDoubleClicked(const FAssetData& AssetData);
	/**
	 * @brief Callback executed when the References button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnReferenceViewerClicked();
	/**
	 * @brief Callback executed when the Audit button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnAuditClicked();
	/**
	 * @brief Callback executed when the Delete button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnDeleteClicked();
	/**
	 * @brief Callback executed when the Whitelist button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnWhitelistClicked();
	/**
	 * @brief Callback executed when the Blacklist button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnBlacklistClicked();

	/**
	 * @brief Opens the Reference viewer with the input assets
	 * @param AssetsToViewReferences Assets to view references for
	 */
	void ReferenceViewerAssets(const TArray<FAssetData> AssetsToViewReferences);
	/**
	 * @brief Opens the Asset Audit window with the input assets
	 * @param AssetsToAudit Assets to view in the audit
	 */
	void AuditAssets(const TArray<FAssetData> AssetsToAudit);
	/**
	 * @brief Opens a deletion dialog with the input assets
	 * @param AssetsToDelete Assets to propose for deletion
	 */
	void DeleteAssets(const TArray<FAssetData> AssetsToDelete);
	/**
	 * @brief Adds input assets to the plugin setting's Whitelist
	 * @param AssetsToWhitelist Assets to add to the list
	 */
	void WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist);
	/**
	 * @brief Adds input assets to the plugin setting's Blacklist
	 * @param AssetsToBlacklist Assets to add to the list
	 */
	void BlackListAssets(const TArray<FAssetData> AssetsToBlacklist);

	/**
	 * @brief Determine if a certain Asset should be displayed inside the list
	 * @param AssetData Asset we are evaluating
	 * @return true to hide the asset, false to show the asset
	 */
	bool FilterDisplayedAsset(const FAssetData& AssetData) const;
	void RemoveFromList(const TArray<FAssetData> AssetsToRemove);
	void RefreshAssetList();

	/**
	 * @brief Determines what assets we want to perform the current operation on.
	 * @return if no assets are selected -> all assets in report. If at least one asset is selected -> current selection
	 */
	TArray<FAssetData> GetAssetsForAction() const;
	/**
	 * @brief Sums up the size of all the assets in the list
	 * @param AssetsList Assets to sum up the size for
	 * @return Sum of all the assets' sizes (in MB)
	 */
	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList) const;
	/**
	 * @brief Determines the size on disk of an asset
	 * @param Asset Asset to check size for
	 * @return Size of asset in MB
	 */
	int64 GetAssetDiskSize(const FAssetData& Asset) const;

	/**
	 * @brief Assets displayed in the report. This can change through the window's lifetime
	 */
	TArray<FAssetData> ReportAssets;
	/**
	 * @brief Delegate bound to the asset picker's properties. Execute it to get the currently selected assets
	 */
	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate;
	
	FRefreshAssetViewDelegate RefreshAssetViewDelegate;
};
