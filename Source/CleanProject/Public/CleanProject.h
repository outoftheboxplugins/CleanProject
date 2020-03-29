// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "AssetData.h"

#include "Runtime/Launch/Resources/Version.h"

/**
 * Implements the CleanProjectEditor module.
 */
class FCleanProjectModule : public IModuleInterface
{

//~ IModuleInterface interface
public:	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// Main menu
	void RegisterMenus();

	// Content browser action
	void CreateContentBrowserEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	TSharedRef<FExtender> CreateContentBrowserExtender(const TArray<FAssetData>& SelectedAssets);

private:
    TSharedPtr<FExtender> MenuExtender;
};
