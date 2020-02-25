// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.


#include "SCleanProjectAssetDialog.h"
#include "Interfaces/IMainFrameModule.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCleanProjectAssetDialog::Construct(const FArguments& InArgs)
{
	
}

void SCleanProjectAssetDialog::OpenAssetDialog(const TArray<FAssetData>& AssetsToReport)
{
	TSharedRef<SWindow> ReportWindow = SNew(SWindow)
		.Title(LOCTEXT("CleanProject_AssetDialogTitle", "Clean Project Analyser"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SCleanProjectAssetDialog)
		];

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	if (MainFrameModule.GetParentWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(ReportWindow, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(ReportWindow);
	}
}

void SCleanProjectAssetDialog::CloseDialog()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());

	if (Window.IsValid())
	{
		Window->RequestDestroyWindow();
	}
}

#undef LOCTEXT_NAMESPACE

// Prompt the user displaying all assets that are going to be deleted.
//const FText ReportMessage = LOCTEXT("DepenCheckerReportTitle", "The following assets are not used by the selected assets.");
//TArray<FString> ReportPackageNames;
//for (auto PackageIt = DependenciesToTest.CreateConstIterator(); PackageIt; ++PackageIt)
//{
//	ReportPackageNames.Add((*PackageIt).PackageName.ToString());
//}

//SDependReportDialog::FOnReportConfirmed OnReportConfirmed = SPackageReportDialog::FOnReportConfirmed::CreateRaw(this, &FCleanProjectModule::CheckDepencies_ReportConfirmed, DependenciesToTest);
//SDependReportDialog::FOnReportConfirmed OnReporBlackListed = SPackageReportDialog::FOnReportConfirmed::CreateRaw(this, &FCleanProjectModule::CheckDepencies_ReportBlackListed, DependenciesToTest);
//SDependReportDialog::OpenDependReportDialog(ReportMessage, ReportPackageNames, OnReportConfirmed, OnReporBlackListed);
//
//
//if (FModuleManager::Get().ModuleExists(TEXT("AssetManagerEditor")))
//{
//	IAssetManagerEditorModule& Module = FModuleManager::LoadModuleChecked< IAssetManagerEditorModule >("AssetManagerEditor");
//	Module.OpenAssetAuditUI(DependenciesToTest);
//
//}
//else {
//	UE_LOG(LogTemp, Error, TEXT("AssetManagerEditor plugin is not enabled"));
//}