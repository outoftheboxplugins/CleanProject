// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

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

private:
	FDelegateHandle CBAssetsExtenderDelegateHandle;
	FDelegateHandle CBFoldersExtenderDelegateHandle;
};
