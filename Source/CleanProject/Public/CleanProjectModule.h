// Copyright Out-of-the-Box Plugins 2018-2020. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Implements the CleanProject editor module.
 */
class FCleanProjectModule : public IModuleInterface
{
	// IModuleInterface interface
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Register & Unregister
private:
	void RegisterMenus();
	void UnregisterMenus();

	void RegisterSettings();
	void UnregisterSettings();

	void RegisterAssetActions();
	void UnregisterAssetActions();

private:
	TSharedPtr<FExtender> MenuExtender;
	FDelegateHandle CBExtenderDelegateHandle;
};

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject);
