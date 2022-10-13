// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPUnusedAssetsReport.h"

#include "AssetManagerEditorModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CPLog.h"
#include "CPSettings.h"
#include "ContentBrowserModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IContentBrowserSingleton.h"
#include "Interfaces/IMainFrameModule.h"
#include "ObjectTools.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPUnusedAssetsReport::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ReportAssets = AssetsToReport;

	const int64 TotalDiskSize = GetAssetsDiskSize(ReportAssets);

	// clang-format off
	ChildSlot
	[
		SNew(SVerticalBox)

		// Titlebar
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text( FText::Format(LOCTEXT("ReportDiskSize", "Total disk size: {0}"), FText::AsMemory(TotalDiskSize)))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReportSubtitle", "The following assets were found unused:"))
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
				CreateAssetPickerWidget()
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
				.OnClicked(this, &SCPUnusedAssetsReport::OnDeleteClicked)
				.Text(LOCTEXT("DeleteButton", "Delete"))
			]
			+SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnMoreInfoClicked)
				.Text(LOCTEXT("MoreInfoButton", "More Info"))
			]
			+SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnWhitelistClicked)
				.Text(LOCTEXT("WhitelistButton", "Whitelist"))
			]
			+SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnBlacklistClicked)
				.Text(LOCTEXT("BlacklistButton", "Blacklist"))
			]
			+SUniformGridPanel::Slot(4, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnCancelClicked)
				.Text(LOCTEXT("CancelButton", "Cancel"))
			]
		]
	];
	// clang-format on
}

//////////////////////////////////////////////////////////////////////////
// Window commands

void SCPUnusedAssetsReport::OpenAssetDialog(const TArray<FAssetData>& AssetsToReport)
{
	// clang-format off
	const TSharedRef<SWindow> ReportWindow = SNew(SWindow)
		.Title(LOCTEXT("AssetDialogTitle", "Clean Project - Delete Preview"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SCPUnusedAssetsReport, AssetsToReport)
		];
	// clang-format on

	FSlateApplication::Get().AddWindow(ReportWindow);
}

void SCPUnusedAssetsReport::CloseAssetDialog()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());

	if (Window.IsValid())
	{
		Window->RequestDestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////////
// Slate Delegates

TSharedPtr<SWidget> SCPUnusedAssetsReport::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	// TODO: Add reference viewer
	// TODO: Add show in content browser

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(TEXT("ReportContextMenu"), LOCTEXT("ReportConextMenuCategory", "Cleanup actions"));

	MenuBuilder.AddMenuEntry(LOCTEXT("RemoveAction", "Remove"),
		LOCTEXT("RemoveActionTooltip", "Remove selected assets from the report, so they won't get deleted."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::RemoveFromList, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("MoreInfoAction", "More Info"),
		LOCTEXT("MoreInfoTooltip", "Get more information about the selected assets."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::MoreInfoAsset, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("WhitelistAction", "Whitelist"),
		LOCTEXT("WhitelistActionTooltip", "Whitelist only selected assets and remove from report."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::WhiteListAssets, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("BlacklistAction", "Blacklist"),
		LOCTEXT("BlacklistActionTooltip", "Blacklist only selected assets and remove from report."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::BlackListAssets, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("DeleteAction", "Delete"), LOCTEXT("DeleteActionTooltip", "Delete only selected assets."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.Delete"),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::DeleteAssets, SelectedAssets)));

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void SCPUnusedAssetsReport::OnRequestOpenAsset(const FAssetData& AssetData)
{
	TArray<FName> AssetNames;
	AssetNames.Add(AssetData.PackageName);

	IAssetManagerEditorModule& ManagerEditorModule = IAssetManagerEditorModule::Get();
	ManagerEditorModule.OpenReferenceViewerUI(AssetNames);
}

//////////////////////////////////////////////////////////////////////////
// Buttons Actions

FReply SCPUnusedAssetsReport::OnDeleteClicked()
{
	DeleteAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnMoreInfoClicked()
{
	MoreInfoAsset(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnWhitelistClicked()
{
	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->WhitelistAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnBlacklistClicked()
{
	BlackListAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnCancelClicked()
{
	CloseAssetDialog();
	return FReply::Handled();
}

//////////////////////////////////////////////////////////////////////////
// Functionality

void SCPUnusedAssetsReport::DeleteAssets(const TArray<FAssetData> AssetsToDelete)
{
	LOG_TRACE();

	TArray<UObject*> ObjectsToDelete;
	for (const FAssetData& AssetData : AssetsToDelete)
	{
		ObjectsToDelete.Add(AssetData.GetAsset());
	}

	ObjectTools::DeleteObjects(ObjectsToDelete);
	RemoveFromList(AssetsToDelete);
}

void SCPUnusedAssetsReport::MoreInfoAsset(const TArray<FAssetData> AssetsToGetInfo)
{
	LOG_TRACE();

	if (FModuleManager::Get().ModuleExists(TEXT("AssetManagerEditor")))
	{
		TArray<FName> AssetNames;

		for (const FAssetData& AssetData : AssetsToGetInfo)
		{
			AssetNames.Add(AssetData.PackageName);
		}

		IAssetManagerEditorModule& Module = FModuleManager::LoadModuleChecked<IAssetManagerEditorModule>("AssetManagerEditor");
		Module.OpenAssetAuditUI(AssetNames);
	}
}

void SCPUnusedAssetsReport::BlackListAssets(const TArray<FAssetData> AssetsToBlacklist)
{
	LOG_TRACE();

	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->BlacklistAssets(AssetsToBlacklist);

	RemoveFromList(AssetsToBlacklist);
}

void SCPUnusedAssetsReport::WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist)
{
	LOG_TRACE();

	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->WhitelistAssets(AssetsToWhitelist);

	RemoveFromList(AssetsToWhitelist);
}

void SCPUnusedAssetsReport::RemoveFromList(const TArray<FAssetData> AssetsToRemove)
{
	LOG_TRACE();

	ReportAssets.RemoveAllSwap([&AssetsToRemove](const FAssetData& AssetData) { return AssetsToRemove.Contains(AssetData); });

	if (ReportAssets.Num() == 0)
	{
		CloseAssetDialog();
	}
	else
	{
		TArray<FName> AssetObjectPaths;
		AssetObjectPaths.Reserve(AssetsToRemove.Num());

		for (const FAssetData& AssetData : AssetsToRemove)
		{
			AssetObjectPaths.Add(AssetData.ObjectPath);
		}

		ReportAssetsFilter.ObjectPaths.RemoveAllSwap(
			[&AssetObjectPaths](const FName& objectPath) { return AssetObjectPaths.Contains(objectPath); });

		SetFilterDelegate.Execute(ReportAssetsFilter);
	}
}

TArray<FAssetData> SCPUnusedAssetsReport::GetAssetsForAction() const
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

TSharedRef<SWidget> SCPUnusedAssetsReport::CreateAssetPickerWidget()
{
	FAssetPickerConfig Config;
	Config.InitialAssetViewType = EAssetViewType::List;
	Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateStatic(OnRequestOpenAsset);
	Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SCPUnusedAssetsReport::OnGetAssetContextMenu);
	Config.SetFilterDelegates.Add(&SetFilterDelegate);
	Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);

	TArray<FName>& ReportObjectsPaths = ReportAssetsFilter.ObjectPaths;
	ReportObjectsPaths.Reserve(ReportAssets.Num());

	for (auto PackageIt = ReportAssets.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		ReportObjectsPaths.Add(PackageIt->ObjectPath);
	}

	Config.Filter = ReportAssetsFilter;

	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TSharedRef<SWidget> AssetPickerWidget = ContentBrowserModule.Get().CreateAssetPicker(Config);
	return AssetPickerWidget;
}

int64 SCPUnusedAssetsReport::GetAssetDiskSize(const FAssetData& Asset)
{
	const FName PackageName = FName(*Asset.GetPackage()->GetName());
	TOptional<FAssetPackageData> PackageData = FAssetRegistryModule::GetRegistry().GetAssetPackageDataCopy(PackageName);
	if (PackageData.IsSet())
	{
		return PackageData.GetValue().DiskSize;
	}
	return -1;
}

int64 SCPUnusedAssetsReport::GetAssetsDiskSize(const TArray<FAssetData>& AssetsList)
{
	int64 TotalDiskSize = 0;
	for (const FAssetData& AssetDataReported : AssetsList)
	{
		const FText CurrentAssetName = FText::FromName(AssetDataReported.PackageName);
		const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
		TotalDiskSize += GetAssetDiskSize(AssetDataReported);
	}

	return TotalDiskSize;
}

#undef LOCTEXT_NAMESPACE
