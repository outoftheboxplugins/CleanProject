// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

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
	static void OpenDialog(const TArray<FAssetData>& AssetsToReport);

	SLATE_BEGIN_ARGS(SCPUnusedAssetsReport) {}
	SLATE_END_ARGS()

	/**
	 * @brief Creates the slate widget that represents the Clean Project Report screen
	 * @param InArgs A set of slate arguments, defined above.
	 * @param AssetsToReport Assets this report should display
	 */
	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);

private:
	/**
	 * @brief Creates asset picker widget and binds delegates for future callbacks
	 * @return Slate widget reference
	 */
	TSharedRef<SWidget> CreateAssetPickerWidget();
	/**
	 * @brief Callback executed to build the context menu for the selected asset(s)
	 * @param SelectedAssets Current selection of assets we want to get available actions for
	 * @return Widget to be displayed as context menu
	 */
	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets);
	/**
	 * @brief Callback executed when an asset is double clicked
	 * @param AssetData Clicked asset
	 */
	void OnAssetDoubleClicked(const FAssetData& AssetData);
	/**
	 * @brief Callback executed when the Delete button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnDeleteClicked();
	/**
	 * @brief Callback executed when the Mark as Core button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnMarkAsCoreClicked();
	/**
	 * @brief Callback executed when the Exclude from Package button is clicked
	 * @return if the operation was handled or not
	 */
	FReply OnExcludeFromPackageClicked();
	/**
	 * @brief Opens the Reference viewer with the input assets
	 * @param Assets Assets to view references for
	 */
	void ReferenceViewerAssets(const TArray<FAssetData> Assets);
	/**
	 * @brief Opens the Asset Audit window with the input assets
	 * @param Assets Assets to view in the audit
	 */
	void AuditAssets(const TArray<FAssetData> Assets);
	/**
	 * @brief Removes assets from Source control 'Mark for Add' state without deleting them
	 * @param Assets Assets to revert
	 */
	void SoftRevertFiles(const TArray<FAssetData> Assets);
	/**
	 * @brief Opens a deletion dialog with the input assets
	 * @param Assets Assets to propose for deletion
	 */
	void DeleteAssets(const TArray<FAssetData> Assets);
	/**
	 * @brief Mark the input assets as Core inside the plugin's settings
	 * @param Assets Assets to mark as Core
	 */
	void MarkAssetsAsCore(const TArray<FAssetData> Assets);
	/**
	 * @brief Exclude th input assets from the package inside the plugin's settings
	 * @param Assets Assets to exclude from package
	 */
	void ExcludeAssetsFromPackage(const TArray<FAssetData> Assets);
	/**
	 * @brief Determine if a certain Asset should be displayed inside the list
	 * @param AssetData Asset we are evaluating
	 * @return true to hide the asset, false to show the asset
	 */
	bool FilterDisplayedAsset(const FAssetData& AssetData) const;
	/**
	 * @brief Remove assets from the report for various reasons (removed from list by the user, already deleted)
	 * @param Assets Assets we want to remove from the report
	 */
	void RemoveFromList(const TArray<FAssetData> Assets);
	/**
	 * @brief Determines what assets we want to perform the current operation on.
	 * @return if no assets are selected -> all assets in report. If at least one asset is selected -> current selection
	 */
	TArray<FAssetData> GetAssetsForAction() const;
	/**
	 * @brief Sums up the size of all the assets in the list
	 * @param Assets Assets to sum up the size for
	 * @return Sum of all the assets' sizes (in MB)
	 */
	int64 GetAssetsDiskSize(const TArray<FAssetData>& Assets) const;
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
	/**
	 * @brief Delegate bound to the asset picker's properties. Execute it to refresh the assets displayed
	 */
	FRefreshAssetViewDelegate RefreshAssetViewDelegate;
};
