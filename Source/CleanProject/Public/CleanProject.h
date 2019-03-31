// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Implements the CleanProjectEditor module.
 */
class FCleanProjectModule : public IModuleInterface
{
public:
	//~ IModuleInterface interface

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

protected:
	//~ Functionality
	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies) const;
	void DepenChecker(TArray<FAssetData> SelectedAssets);
	void CheckDepencies_ReportConfirmed(TArray<FAssetData> ConfirmedPackageNamesToDelete) const;
};
