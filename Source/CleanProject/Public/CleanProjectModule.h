// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

class FCleanProjectModule : public IModuleInterface
{

// IModuleInterface interface
private:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
void MakeCleanProjectActionsMenu(UToolMenu* InMenu);

// Register & Unregister
private:
	void RegisterMenus();

	void RegisterAssetActions();
	void UnregisterAssetActions();

	void RegisterMenuSpawner();
	void UnregisterMenuSpawner();

private:
	FDelegateHandle CBAssetsExtenderDelegateHandle;
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};

//TODO: Plugin to alt tab assets, see recents, open last closed -> experience similar to browser
//TODO: right click asset -> explicitly cook / add to maps to package
