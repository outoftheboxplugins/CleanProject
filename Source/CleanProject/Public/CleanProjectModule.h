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

private:
	TSharedPtr<FExtender> MenuExtender;
};

IMPLEMENT_MODULE(FCleanProjectModule, CleanProject);

/*	
	// Content browser action
	void CreateContentBrowserEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	TSharedRef<FExtender> CreateContentBrowserExtender(const TArray<FAssetData>& SelectedAssets);
};
*/