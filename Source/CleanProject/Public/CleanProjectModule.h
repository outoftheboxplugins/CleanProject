// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "CPMenuExtensions.h"

/**
 * Implements the CleanProject editor module.
 */

class FCleanProjectModule : public IModuleInterface
{

// IModuleInterface interface
private:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

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
