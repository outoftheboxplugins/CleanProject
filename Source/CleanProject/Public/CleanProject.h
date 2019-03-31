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
	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CreateDataValidationContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void DepenChecker(TArray<FAssetData> SelectedAssets);
	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies) const;
	void CheckDepencies_ReportConfirmed(TArray<FAssetData> ConfirmedPackageNamesToDelete) const;
	void Menu_DepenChecker();
	void DepenCheckerMenuCreationDelegate(FMenuBuilder& MenuBuilder);

private:
	TSharedPtr<FExtender> MenuExtender;

	FDelegateHandle ContentBrowserAssetExtenderDelegateHandle;
};
