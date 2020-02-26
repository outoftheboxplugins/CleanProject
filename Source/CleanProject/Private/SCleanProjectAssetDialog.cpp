// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.


#include "SCleanProjectAssetDialog.h"
#include "Interfaces/IMainFrameModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/Public/Widgets/Layout/SUniformGridPanel.h"
#include "Slate/Public/Widgets/Input/SButton.h"
#include "Slate/Public/Widgets/Text/STextBlock.h"
#include "EditorStyle/Public/EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCleanProjectAssetDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ReportAssets = AssetsToReport;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Titlebar
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CleanProject_ReportSubtitle", "The following assets were found unused:"))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		// Tree of packages in the report
		+ SVerticalBox::Slot()
		//.FillHeight(1.f)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			//[
			//	SAssignNew(ReportTreeView, DependReportTree)
			//	.TreeItemsSource(&DependReportRootNode.Children)
			//	.ItemHeight(18)
			//	.SelectionMode(ESelectionMode::Single)
			//	.OnGenerateRow(this, &SDependReportDialog::GenerateTreeRow)
			//	.OnGetChildren(this, &SDependReportDialog::GetChildrenForTree)
			//]
		]

		// Buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(4, 4)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
			
			+SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCleanProjectAssetDialog::OnDeleteClicked)
				.Text(LOCTEXT("CleanProject_DeleteButton", "Delete"))
			]
			+SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCleanProjectAssetDialog::OnAuditClicked)
				.Text(LOCTEXT("CleanProject_AuditButton", "More Info"))
			]
			+SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCleanProjectAssetDialog::OnBlacklistClicked)
				.Text(LOCTEXT("CleanProject_BlacklistButton", "Blacklist"))
			]
			+SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCleanProjectAssetDialog::OnCancelClicked)
				.Text(LOCTEXT("CleanProject_CancelButton", "Cancel"))
			]
		]
	];
}

void SCleanProjectAssetDialog::OpenAssetDialog(const TArray<FAssetData>& AssetsToReport)
{
	TSharedRef<SWindow> ReportWindow = SNew(SWindow)
		.Title(LOCTEXT("CleanProject_AssetDialogTitle", "Clean Project Analyser"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SCleanProjectAssetDialog, AssetsToReport)
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

FReply SCleanProjectAssetDialog::OnDeleteClicked()
{
	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnAuditClicked()
{
	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnBlacklistClicked()
{
	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnCancelClicked()
{
	return FReply::Handled();
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