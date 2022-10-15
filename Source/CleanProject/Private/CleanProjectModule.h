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
	void RegisterToolActions();
	void CreateToolActionEntries(UToolMenu* InMenu);
	void UnregisterToolActions();

	void RegisterAssetActions();
	void UnregisterAssetActions();

	void RegisterToolWindows();
	void UnregisterToolWindows();

	TSharedRef<FExtender> CreateContentBrowserAssetsExtender(const TArray<FAssetData>& SelectedAssets);
	void CreateContentBrowserAssetsEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

	TSharedRef<FExtender> CreateContentBrowserFoldersExtender(const TArray<FString>& SelectedFolders);
	void CreateContentBrowserFoldersEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedFolders);

private:
	FDelegateHandle CBAssetsExtenderDelegateHandle;
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};
