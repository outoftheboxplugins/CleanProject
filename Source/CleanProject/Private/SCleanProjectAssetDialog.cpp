// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.


#include "SCleanProjectAssetDialog.h"
#include "Interfaces/IMainFrameModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/Public/Widgets/Layout/SUniformGridPanel.h"
#include "Slate/Public/Widgets/Input/SButton.h"
#include "Slate/Public/Widgets/Text/STextBlock.h"
#include "EditorStyle/Public/EditorStyleSet.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "CleanProjectSettings.h"
#include "Misc/FileHelper.h"
#include "AssetManagerEditorModule.h"
#include "IAssetRegistry.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"

#include "Framework/Commands/UIAction.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "EditorStyleSet.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SViewport.h"
#include "FileHelpers.h"
#include "ARFilter.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetRegistryModule.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "FrontendFilterBase.h"
#include "Slate/SceneViewport.h"
#include "ObjectEditorUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/BlueprintCore.h"
#include "Widgets/Input/SComboBox.h"
#include "Framework/Application/SlateApplication.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Blueprint/BlueprintSupport.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "ContentBrowser/Private/SAssetDialog.h"
#include "ContentBrowser/Private/SAssetPicker.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCleanProjectAssetDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ReportAssets.Append(AssetsToReport);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	IAssetManagerEditorModule& ManagerEditorModule = IAssetManagerEditorModule::Get();

	FAssetPickerConfig Config;
	{
		Config.InitialAssetViewType = EAssetViewType::Column;
		Config.bAddFilterUI = true;
		Config.bShowPathInColumnView = true;
		Config.bSortByPathInColumnView = true;

		// Configure response to click and double-click
		//Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SAssetAuditBrowser::OnRequestOpenAsset);
		//Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SAssetAuditBrowser::OnGetAssetContextMenu);
		//Config.OnAssetTagWantsToBeDisplayed = FOnShouldDisplayAssetTag::CreateSP(this, &SAssetAuditBrowser::CanShowColumnForAssetRegistryTag);
		//Config.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SAssetAuditBrowser::HandleFilterAsset);
		//Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
		//Config.SetFilterDelegates.Add(&SetFilterDelegate);

		Config.bFocusSearchBoxWhenOpened = false;
		Config.bPreloadAssetsForContextMenu = false;

		// Hide path and type by default
		auto Settings = GetDefault<UCleanProjectSettings>();
		Config.HiddenColumnNames.Append(Settings->ReportHiddenColumns);
		
		// Add custom columns
		Config.CustomColumns.Emplace(IAssetManagerEditorModule::DiskSizeName, 
			LOCTEXT("CleanProject_SizeColumn", "Disk Size"),
			LOCTEXT("CleanProject_SizeColumnTooltip", "Size of saved file on disk for only this asset"), 
			UObject::FAssetRegistryTag::TT_Numerical, 
			FOnGetCustomAssetColumnData::CreateSP(this, &SCleanProjectAssetDialog::GetDiskSizeData),
			FOnGetCustomAssetColumnDisplayText::CreateSP(this, &SCleanProjectAssetDialog::GetDiskSizeDisplayText));
	}

	TArray<FName>& ReportObjectsPaths = Config.Filter.ObjectPaths;
	ReportObjectsPaths.Reserve(ReportAssets.Num());

	for (auto PackageIt = ReportAssets.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		ReportObjectsPaths.Add(PackageIt->ObjectPath);
	}

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
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(Config)
			]
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

	ManagerEditorModule.RefreshRegistryData();
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

FString SCleanProjectAssetDialog::GetDiskSizeData(FAssetData& AssetData, FName ColumnName) const
{
	FName packageName = FName(*AssetData.GetPackage()->GetName());

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetPackageData* packageData = AssetRegistryModule.Get().GetAssetPackageData(packageName);

	if (packageData)
	{
		return LexToString(packageData->DiskSize);
	}
	else
	{
		return FString::FString("Invalid");
	}
}

FText SCleanProjectAssetDialog::GetDiskSizeDisplayText(FAssetData& AssetData, FName ColumnName) const
{
	FName packageName = FName(*AssetData.GetPackage()->GetName());

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetPackageData* packageData = AssetRegistryModule.Get().GetAssetPackageData(packageName);

	if (packageData)
	{
		return FText::AsMemory(packageData->DiskSize);
	}
	else
	{
		return LOCTEXT("CleanProject_UnkownSize", "Invalid Size");
	}
}

FReply SCleanProjectAssetDialog::OnDeleteClicked()
{
	TArray<UObject*> AssetsToDelete;
	for (auto AssetIt = ReportAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		AssetsToDelete.Add(AssetIt->GetAsset());
	}

	ObjectTools::DeleteObjects(AssetsToDelete);

	CloseDialog();

	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnAuditClicked()
{
	if (FModuleManager::Get().ModuleExists(TEXT("AssetManagerEditor")))
	{
		TArray<FName> AssetNames;

		for (auto PackageIt = ReportAssets.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			AssetNames.Add(PackageIt->PackageName);
		}

		IAssetManagerEditorModule& Module = FModuleManager::LoadModuleChecked< IAssetManagerEditorModule >("AssetManagerEditor");
		Module.OpenAssetAuditUI(AssetNames);
	}

	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnBlacklistClicked()
{
	FString FileContent;
	for (auto PackageIt = ReportAssets.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		FString assetPath = PackageIt->PackageName.ToString();
		FileContent += FString::Printf(TEXT("../../..%s\n"), *assetPath);
	}

	auto Settings = GetDefault<UCleanProjectSettings>();
	if (Settings->bUseSmartBlackList)
	{
		FString projectBuildRoot = FPaths::ProjectDir() + "Build";

		for (const FString& platformFolder : Settings->PlatformsPaths)
		{
			for (const FString& listFile : Settings->BlacklistFiles)
			{
				FString slash = FGenericPlatformMisc::GetDefaultPathSeparator();
				FString platformPath = projectBuildRoot + slash + platformFolder + slash + listFile;
				FFileHelper::SaveStringToFile(FileContent, *platformPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
			}
		}
	}
	else
	{
		FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Blacklist.txt");

		FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
		FPlatformProcess::LaunchURL(*FString::Printf(TEXT("file://%s"), *FilePath), NULL, NULL);
	}

	CloseDialog();

	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnCancelClicked()
{
	CloseDialog();

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

//else {
//	UE_LOG(LogTemp, Error, TEXT("AssetManagerEditor plugin is not enabled"));
//}