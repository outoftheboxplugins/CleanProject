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
#include "Editor.h"
#include "ContentBrowser/Private/SAssetDialog.h"
#include "ContentBrowser/Private/SAssetPicker.h"
#include "ContentBrowser/Private/SAssetView.h"
#include "CleanProjectGameSettings.h"
#include "SCleanProjectBlacklistDialog.h"
#include "CleanProjectOperations.h"
#include "CleanProject.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCleanProjectAssetDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ReportAssets.Append(AssetsToReport);
	int64 TotalDiskSize = 0;

	for (const FAssetData& AssetDataReported : AssetsToReport)
	{
		TotalDiskSize += GetAssetDiskSize(AssetDataReported);
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	IAssetManagerEditorModule& ManagerEditorModule = IAssetManagerEditorModule::Get();

	FAssetPickerConfig Config;
	{
		Config.InitialAssetViewType = EAssetViewType::Column;
		Config.bAddFilterUI = true;
		Config.bShowPathInColumnView = true;
		Config.bSortByPathInColumnView = true;

		// Configure response to double-click and context-menu
		Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SCleanProjectAssetDialog::OnRequestOpenAsset);
		Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SCleanProjectAssetDialog::OnGetAssetContextMenu);
		Config.SetFilterDelegates.Add(&SetFilterDelegate);
		Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);

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

	TArray<FName>& ReportObjectsPaths = ReportAssetsFilter.ObjectPaths;
	ReportObjectsPaths.Reserve(ReportAssets.Num());

	for (auto PackageIt = ReportAssets.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		ReportObjectsPaths.Add(PackageIt->ObjectPath);
	}

	Config.Filter = ReportAssetsFilter;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Titlebar
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text( FText::Format(LOCTEXT("CleanProject_ReportDiskSize", "Total disk size: {0}"), FText::AsMemory(TotalDiskSize)))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		+ SVerticalBox::Slot()
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

#ifdef CLEANPROJECT_COMPATIBILITY
	ManagerEditorModule.RefreshRegistryData();
#endif // CLEANPROJECT_COMPATIBILITY
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

TSharedPtr<SWidget> SCleanProjectAssetDialog::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/ true, NULL);
	
	MenuBuilder.BeginSection(TEXT("CleanProject_ReportContextMenu"),
		LOCTEXT("CleanProject_ReportConextMenuCategory", "Cleanup actions"));
	
	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProject_RemoveAction", "Remove"),
		LOCTEXT("CleanProject_RemoveActionTooltip", "Remove selected assets from the report, so they won't get deleted."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SCleanProjectAssetDialog::RemoveFromList, SelectedAssets),
			FCanExecuteAction()
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProject_AuditAction", "Audit"),
		LOCTEXT("CleanProject_AuditActionTooltip", "Get more information about the selected assets."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SCleanProjectAssetDialog::AuditAssets, SelectedAssets),
			FCanExecuteAction()
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProject_BlacklistAction", "Blacklist"),
		LOCTEXT("CleanProject_BlacklistActionTooltip", "Blacklist only selected assets and remove from report."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SCleanProjectAssetDialog::BlackListAssets, SelectedAssets),
			FCanExecuteAction()
		));
	
	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProject_WhitelistAction", "Whitelist"),
		LOCTEXT("CleanProject_WhitelistActionTooltip", "Whitelist only selected assets and remove from report."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SCleanProjectAssetDialog::WhiteListAssets, SelectedAssets),
			FCanExecuteAction()
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CleanProject_DeleteAction", "Delete"),
		LOCTEXT("CleanProject_DeleteActionTooltip", "Delete only selected assets."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.Delete"),
		FUIAction(
			FExecuteAction::CreateSP(this, &SCleanProjectAssetDialog::DeleteAssets, SelectedAssets),
			FCanExecuteAction()
		));

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void SCleanProjectAssetDialog::OnRequestOpenAsset(const FAssetData& AssetData) const
{
	TArray<FName> AssetNames;
	AssetNames.Add(AssetData.PackageName);

    IAssetManagerEditorModule& ManagerEditorModule = IAssetManagerEditorModule::Get();
	ManagerEditorModule.OpenReferenceViewerUI(AssetNames);
}

int64 SCleanProjectAssetDialog::GetAssetDiskSize(const FAssetData& Asset) const
{
	if (Asset.GetPackage() == nullptr)
	{
		return 0;
	}

	FName packageName = FName(*Asset.GetPackage()->GetName());

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetPackageData* packageData = AssetRegistryModule.Get().GetAssetPackageData(packageName);

	if (packageData)
	{
		return packageData->DiskSize;
	}
	else
	{
		return -1;
	}
}

FString SCleanProjectAssetDialog::GetDiskSizeData(FAssetData& AssetData, FName ColumnName) const
{
	const int64 DiskSize = GetAssetDiskSize(AssetData);

	if (DiskSize > 0)
	{
		return LexToString(DiskSize);
	}
	else
	{
		return FString("Invalid");
	}
}

FText SCleanProjectAssetDialog::GetDiskSizeDisplayText(FAssetData& AssetData, FName ColumnName) const
{
	const int64 DiskSize = GetAssetDiskSize(AssetData);

	if (DiskSize > 0)
	{
		return FText::AsMemory(DiskSize);
	}
	else
	{
		return LOCTEXT("CleanProject_UnkownSize", "Invalid Size");
	}
}

FReply SCleanProjectAssetDialog::OnDeleteClicked()
{
	DeleteAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnAuditClicked()
{
	AuditAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnBlacklistClicked()
{
	BlackListAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCleanProjectAssetDialog::OnCancelClicked()
{
	CloseDialog();
	return FReply::Handled();
}

void SCleanProjectAssetDialog::DeleteAssets(const TArray<FAssetData> AssetsToDelete)
{
	TArray<UObject*> ObjectsToDelete;
	for(const FAssetData& AssetData : AssetsToDelete)
	{
		ObjectsToDelete.Add(AssetData.GetAsset());
	}

	int32 NumAssetsDeleted = ObjectTools::DeleteObjects(ObjectsToDelete);

	if (NumAssetsDeleted > 0)
	{
		RemoveFromList(AssetsToDelete);
	}
}

void SCleanProjectAssetDialog::AuditAssets(const TArray<FAssetData> AssetsToAudit)
{
	if (FModuleManager::Get().ModuleExists(TEXT("AssetManagerEditor")))
	{
		TArray<FName> AssetNames;

		for(const FAssetData& AssetData: AssetsToAudit)
		{
			AssetNames.Add(AssetData.PackageName);
		}

		IAssetManagerEditorModule& Module = FModuleManager::LoadModuleChecked< IAssetManagerEditorModule >("AssetManagerEditor");
		Module.OpenAssetAuditUI(AssetNames);
	}
}

void SCleanProjectAssetDialog::BlackListAssets(const TArray<FAssetData> AssetsToBlacklist)
{
	bool bRemoveAssets = true;
	auto Settings = GetDefault<UCleanProjectSettings>();
	if (Settings->bShouldSkipBlacklistDialog)
	{
		CleanProjectOperations::GenerateBlacklist(AssetsToBlacklist, Settings->bShouldAppendDefault);
	}
	else
	{
		bRemoveAssets = SCleanProjectBlacklistDialog::OpenBlacklistDialog(AssetsToBlacklist);
	}

	if (bRemoveAssets)
	{
		RemoveFromList(AssetsToBlacklist);
	}
}

void SCleanProjectAssetDialog::WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist)
{
	auto Settings = GetMutableDefault<UCleanProjectGameSettings>();
	Settings->WhitelistAssetes(AssetsToWhitelist);

	RemoveFromList(AssetsToWhitelist);
}

void SCleanProjectAssetDialog::RemoveFromList(const TArray<FAssetData> AssetsToRemove)
{
	ReportAssets.RemoveAllSwap([&AssetsToRemove](const FAssetData& AssetData) 
		{
			return AssetsToRemove.Contains(AssetData);
		});

	TArray<FName> AssetObjectPaths;
	AssetObjectPaths.Reserve(AssetsToRemove.Num());

	for (const FAssetData& AssetData : AssetsToRemove)
	{
		AssetObjectPaths.Add(AssetData.ObjectPath);
	}

	ReportAssetsFilter.ObjectPaths.RemoveAllSwap([&AssetObjectPaths](const FName& objectPath)
		{
			return AssetObjectPaths.Contains(objectPath);
		});

	if (ReportAssets.Num() == 0)
	{
		CloseDialog();
	}
	else
	{
		SetFilterDelegate.Execute(ReportAssetsFilter);
	}
}

TArray<FAssetData> SCleanProjectAssetDialog::GetAssetsForAction() const
{
	TArray<FAssetData> SelectedAssets = GetCurrentSelectionDelegate.Execute();

	if (SelectedAssets.Num())
	{
		return SelectedAssets;
	}
	else
	{
		return ReportAssets;
	}
}

#undef LOCTEXT_NAMESPACE