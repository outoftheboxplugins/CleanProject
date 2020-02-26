// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Containers/Array.h"
#include "AssetData.h"

/**
 * Implements the CleanProjectEditor module.
 */
class FCleanProjectModule : public IModuleInterface
{

//~ IModuleInterface interface
public:	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	//~ Content browser Asset Actions
	TSharedRef<FExtender> OnExtendContentBrowserAssetActions(const TArray<FAssetData>& SelectedAssets);	
	void CreateDepenCheckerContentBrowserAssetAction(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

protected:
	//~ Main Menu Entry
	void OnExtendMainMenu();
	void CreateDepenCheckerMainMenuEntry(FMenuBuilder& MenuBuilder);
};
