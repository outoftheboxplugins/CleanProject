// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

class FCleanProjectModule : public IModuleInterface
{

// IModuleInterface interface
private:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

// Register & Unregister
private:
	void RegisterToolActions();
	void CreateToolActionEntries(UToolMenu* InMenu);
	void UnregisterToolActions();

	void RegisterAssetActions();
	void UnregisterAssetActions();

	void RegisterToolWindows();
	void UnregisterToolWindows();

private:
	FDelegateHandle CBAssetsExtenderDelegateHandle;
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};

//TODO: Plugin to alt tab assets, see recents, open last closed -> experience similar to browser
//TODO: right click asset -> explicitly cook / add to maps to package
