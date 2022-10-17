// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPUnusedAssetsReport.h"

#include "CPLog.h"
#include "CPSettings.h"

#include <AssetManagerEditorModule.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <ContentBrowserModule.h>
#include <IContentBrowserSingleton.h>
#include <ObjectTools.h>
#include <Widgets/Input/SButton.h>

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
		.Padding(4)
		[
			SNew(STextBlock)
			.Text( FText::Format(LOCTEXT("ReportDiskSize", "Total disk size: {0}"), FText::AsMemory(TotalDiskSize)))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReportSubtitle", "The following assets were found unreferenced:"))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		// Tree of packages in the report
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(4)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				CreateAssetPickerWidget()
			]
		]

		// Buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnDeleteClicked)
				.Text(LOCTEXT("DeleteButton", "Delete"))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnReferenceViewerClicked)
				.Text(LOCTEXT("ReferenceViewerButton", "References"))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnAuditClicked)
				.Text(LOCTEXT("AuditButton", "Audit"))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnWhitelistClicked)
				.Text(LOCTEXT("WhitelistButton", "Whitelist"))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnBlacklistClicked)
				.Text(LOCTEXT("BlacklistButton", "Blacklist"))
			]
		]
	];
	// clang-format on
}

void SCPUnusedAssetsReport::OpenAssetDialog(const TArray<FAssetData>& AssetsToReport)
{
	// clang-format off
	const TSharedRef<SWindow> ReportWindow = SNew(SWindow)
		.Title(LOCTEXT("UnusedAssetsReportTitle", "Unused Assets Report"))
		.ClientSize(FVector2D(720, 600))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SCPUnusedAssetsReport, AssetsToReport)
		];
	// clang-format on

	FSlateApplication::Get().AddWindow(ReportWindow);
}

TSharedRef<SWidget> SCPUnusedAssetsReport::CreateAssetPickerWidget()
{
	FAssetPickerConfig Config;
	Config.InitialAssetViewType = EAssetViewType::List;
	Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SCPUnusedAssetsReport::OnAssetDoubleClicked);
	Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SCPUnusedAssetsReport::OnGetAssetContextMenu);
	Config.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SCPUnusedAssetsReport::FilterDisplayedAsset);
	Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
	Config.RefreshAssetViewDelegates.Add(&RefreshAssetViewDelegate);
	Config.bCanShowClasses = false;

	IContentBrowserSingleton& ContentBrowserSingleton =
		FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TSharedRef<SWidget> AssetPickerWidget = ContentBrowserSingleton.CreateAssetPicker(Config);

	return AssetPickerWidget;
}

TSharedPtr<SWidget> SCPUnusedAssetsReport::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(TEXT("ReportContextMenu"), LOCTEXT("ReportConextMenuCategory", "Cleanup actions"));

	MenuBuilder.AddMenuEntry(LOCTEXT("RemoveAction", "Remove"),
		LOCTEXT("RemoveActionTooltip", "Remove selected assets from the report, so they won't get deleted."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::RemoveFromList, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("ReferenceViewerAction", "References"),
		LOCTEXT("ReferenceViewerTooltip", "Get more information about the selected assets."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::ReferenceViewerAssets, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("AuditAction", "Audit"),
		LOCTEXT("AuditTooltip", "Open the Asset Audit tab with the selected assets."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::AuditAssets, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("WhitelistAction", "Whitelist"),
		LOCTEXT("WhitelistActionTooltip", "Whitelist only selected assets and remove from report."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::WhiteListAssets, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("BlacklistAction", "Blacklist"),
		LOCTEXT("BlacklistActionTooltip", "Blacklist only selected assets and remove from report."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::BlackListAssets, SelectedAssets)));

	MenuBuilder.AddMenuEntry(LOCTEXT("DeleteAction", "Delete"), LOCTEXT("DeleteActionTooltip", "Delete only selected assets."),
		FSlateIcon(), FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::DeleteAssets, SelectedAssets)));

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void SCPUnusedAssetsReport::OnAssetDoubleClicked(const FAssetData& AssetData)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
}

FReply SCPUnusedAssetsReport::OnReferenceViewerClicked()
{
	ReferenceViewerAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnAuditClicked()
{
	AuditAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnDeleteClicked()
{
	DeleteAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnWhitelistClicked()
{
	WhiteListAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnBlacklistClicked()
{
	BlackListAssets(GetAssetsForAction());
	return FReply::Handled();
}

void SCPUnusedAssetsReport::ReferenceViewerAssets(const TArray<FAssetData> AssetsToViewReferences)
{
	LOG_TRACE();

	TArray<FName> AssetNames;
	Algo::Transform(AssetsToViewReferences, AssetNames, [](const FAssetData& AssetData) { return AssetData.PackageName; });

	IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetNames);
}

void SCPUnusedAssetsReport::AuditAssets(const TArray<FAssetData> AssetsToAudit)
{
	LOG_TRACE();

	IAssetManagerEditorModule::Get().OpenAssetAuditUI(AssetsToAudit);
}

void SCPUnusedAssetsReport::DeleteAssets(const TArray<FAssetData> AssetsToDelete)
{
	LOG_TRACE();

	TArray<UObject*> ObjectsToDelete;
	Algo::Transform(AssetsToDelete, ObjectsToDelete, [](const FAssetData& AssetData) { return AssetData.GetAsset(); });

	// TODO: react to asset deleted delegate because we have no control over what is actually getting deleted
	ObjectTools::DeleteObjects(ObjectsToDelete);
	RemoveFromList(AssetsToDelete);
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

bool SCPUnusedAssetsReport::FilterDisplayedAsset(const FAssetData& AssetData) const
{
	return !ReportAssets.Contains(AssetData);
}

void SCPUnusedAssetsReport::RemoveFromList(const TArray<FAssetData> AssetsToRemove)
{
	ReportAssets.RemoveAllSwap([&AssetsToRemove](const FAssetData& AssetData) { return AssetsToRemove.Contains(AssetData); });

	// Update list of assets displayed
	constexpr bool bUpdateSource = false;
	RefreshAssetViewDelegate.Execute(bUpdateSource);
}

TArray<FAssetData> SCPUnusedAssetsReport::GetAssetsForAction() const
{
	TArray<FAssetData> SelectedAssets = GetCurrentSelectionDelegate.Execute();
	if (SelectedAssets.Num())
	{
		return SelectedAssets;
	}

	return ReportAssets;
}

int64 SCPUnusedAssetsReport::GetAssetsDiskSize(const TArray<FAssetData>& AssetsList) const
{
	int64 TotalDiskSize = 0;
	for (const FAssetData& AssetDataReported : AssetsList)
	{
		TotalDiskSize += GetAssetDiskSize(AssetDataReported);
	}

	return TotalDiskSize;
}

int64 SCPUnusedAssetsReport::GetAssetDiskSize(const FAssetData& Asset) const
{
	TOptional<FAssetPackageData> PackageData = FAssetRegistryModule::GetRegistry().GetAssetPackageDataCopy(Asset.PackageName);
	if (PackageData.IsSet())
	{
		return PackageData.GetValue().DiskSize;
	}
	return -1;
}

#undef LOCTEXT_NAMESPACE
