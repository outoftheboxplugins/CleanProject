// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

/**
 * @brief Clean Project module responsible for registering menus & toolbar extensions
 */
class FCleanProjectModule : public IModuleInterface
{
private:
	// Begin IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface interface

private:
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

	void RegisterToolActions();
	void CreateToolActionEntries(UToolMenu* InMenu);
	void UnregisterToolActions();

	/**
	 * @brief Callback executed by the windows menu entry to spawn the Dashboard nomad tab
	 */
	TSharedRef<SDockTab> CreateDashboardNomadTab(const FSpawnTabArgs& Args);

	TSharedRef<FExtender> CreateContentBrowserAssetsExtender(const TArray<FAssetData>& SelectedAssets);
	void CreateContentBrowserAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

	TSharedRef<FExtender> CreateContentBrowserFoldersExtender(const TArray<FString>& SelectedFolders);
	void CreateContentBrowserFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders);

private:
	/**
	 * @brief Callback handle for the assets context menu extension of the Content Browser
	 */
	FDelegateHandle CBAssetsExtenderDelegateHandle;
	/**
	 * @brief Callback handle for the folders context menu extension of the Content Browser
	 */
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};
