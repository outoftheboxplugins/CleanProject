// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Implements the CleanProject editor module.
 */

class FCleanProjectModule : public IModuleInterface
{
public:
	static void OpenCleanProjectSettings();

// IModuleInterface interface
private:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

// Register & Unregister
private:
	void RegisterMenus();
	void UnregisterMenus();

	void RegisterSettings() const;
	void UnregisterSettings() const;

	void RegisterAssetActions();
	void UnregisterAssetActions();

	void RegisterMenuSpawner();
	void UnregisterMenuSpawner();

private:
	TSharedPtr<FExtender> MenuExtender;

	FDelegateHandle CBAssetsExtenderDelegateHandle;
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject);
