// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

/**
 * @brief Clean Project module responsible for registering menus & toolbar extensions
 */
class FCleanProjectModule : public IModuleInterface
{
	// Begin IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface interface

	/**
	 * @brief Registers all the extensions for the Content Browser module
	 */
	void RegisterContentBrowserExtensions();
	/**
	 * @brief Unregisters all the extensions for the Content Browser module
	 */
	void UnregisterContentBrowserExtensions();
	/**
	 * @brief Registers all the extensions for the Window dropdown menu
	 */
	void RegisterWindowExtensions();
	/**
	 * @brief Unregisters all the extensions for the Window dropdown menu
	 */
	void UnregisterWindowExtensions();
	/**
	 * @brief Registers all the extensions for the Tools dropdown menu
	 */
	void RegisterToolsExtensions();
	/**
	 * @brief Unregisters all the extensions for the Tools dropdown menu
	 */
	void UnregisterToolsExtensions();

	/**
	 * @brief Creates an extender who will add Clean Project actions in the ContentBrowser asset selection context menu
	 * @param SelectedAssets Currently selected assets
	 * @return Reference to the created extender
	 */
	TSharedRef<FExtender> CreateCBAssetsExtender(const TArray<FAssetData>& SelectedAssets);
	/**
	 * @brief Create the entries for the Clean Project actions in the ContentBrowser asset selection context menu
	 * @param MenuBuilder Menu Owner
	 * @param SelectedAssets Current selection of assets
	 */
	void CreateCBAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	/**
	 * @brief Creates an extender who will add Clean project actions in the ContentBrowser folder selection context menu
	 * @param SelectedFolders Currently selected folders
	 * @return Reference to the created extender
	 */
	TSharedRef<FExtender> CreateCBFoldersExtender(const TArray<FString>& SelectedFolders);
	/**
	 * @brief Create the entries for the Clean Project actions in the ContentBrowser folder selection context menu
	 * @param MenuBuilder Menu Owner
	 * @param SelectedFolders Current selection of folders
	 */
	void CreateCBFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders);
	/**
	 * @brief Callback executed by the windows menu entry to spawn the Dashboard nomad tab
	 */
	TSharedRef<SDockTab> CreateDashboardTab(const FSpawnTabArgs& Args);
	/**
	 * @brief Create the entries for the Clean Project actions in the Tools submenu
	 * @param InMenu Menu Owner
	 */
	void CreateToolsSubMenu(UToolMenu* InMenu);
	/**
	 * @brief Callback handle for the assets context menu extension of the Content Browser
	 */
	FDelegateHandle CBAssetsExtenderDelegateHandle;
	/**
	 * @brief Callback handle for the folders context menu extension of the Content Browser
	 */
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};
