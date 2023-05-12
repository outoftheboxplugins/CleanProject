// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPUnusedAssetsReport.h"

#include <AssetManagerEditorModule.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <ContentBrowserModule.h>
#include <IContentBrowserSingleton.h>
#include <ISourceControlModule.h>
#include <ISourceControlProvider.h>
#include <ObjectTools.h>
#include <SourceControlOperations.h>
#include <Widgets/Input/SButton.h>

#include "CPLog.h"
#include "CPSettings.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPUnusedAssetsReport::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ReportAssets = AssetsToReport;
	const int64 TotalDiskSize = GetAssetsDiskSize(ReportAssets);

	const TSharedRef<SWidget> AssetPickerWidget = CreateAssetPickerWidget();

	// clang-format off
	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			SNew(STextBlock)
			.Text( FText::Format(LOCTEXT("ReportDiskSize", "Total disk size: {0}"), FText::AsMemory(TotalDiskSize)))
			.TextStyle(FAppStyle::Get(), "PackageMigration.DialogTitle")
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReportSubtitle", "The following assets were found unreferenced:"))
			.TextStyle(FAppStyle::Get(), "PackageMigration.DialogTitle")
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(4)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				AssetPickerWidget
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
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnDeleteClicked)
				.Text(LOCTEXT("DeleteButton", "Delete"))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnMarkAsCoreClicked)
				.Text(LOCTEXT("MarkAsCoreButton", "Mark As Core"))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPUnusedAssetsReport::OnExcludeFromPackageClicked)
				.Text(LOCTEXT("ExcludeFromPackageButton", "Exclude from Package"))
			]
		]
	];
	// clang-format on
}

void SCPUnusedAssetsReport::OpenDialog(const TArray<FAssetData>& AssetsToReport)
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

	IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TSharedRef<SWidget> AssetPickerWidget = ContentBrowserSingleton.CreateAssetPicker(Config);

	return AssetPickerWidget;
}

TSharedPtr<SWidget> SCPUnusedAssetsReport::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(TEXT("ReportContextMenu"), LOCTEXT("ReportConextMenuCategory", "Cleanup actions"));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("RemoveAction", "Remove"),
		LOCTEXT("RemoveActionTooltip", "Remove selected assets from the report, so they won't get deleted."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::RemoveFromList, SelectedAssets))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ReferenceViewerAction", "References"),
		LOCTEXT("ReferenceViewerTooltip", "Open the References Viewer window with the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::ReferenceViewerAssets, SelectedAssets))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AuditAction", "Audit"),
		LOCTEXT("AuditTooltip", "Open the Asset Audit window with the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::AuditAssets, SelectedAssets))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SoftRevert", "Soft revert"),
		LOCTEXT("SoftRevertTooltip", "Remove selected assets from the 'Mark for Add' state without deleting them."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::SoftRevertFiles, SelectedAssets))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("MarkAsCoreAction", "Mark as Core"),
		LOCTEXT("MarkAsCoreActionTooltip", "Mark selected assets as core and remove them from report."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::MarkAssetsAsCore, SelectedAssets))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ExcludeFromPackageAction", "Exclude from package"),
		LOCTEXT("ExcludeFromPackageTooltip", "Exclude selected assets from package and remove them from report."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::ExcludeAssetsFromPackage, SelectedAssets))
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("DeleteAction", "Delete"),
		LOCTEXT("DeleteActionTooltip", "Initiate a delete action with the selected assets."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SCPUnusedAssetsReport::DeleteAssets, SelectedAssets))
	);

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void SCPUnusedAssetsReport::OnAssetDoubleClicked(const FAssetData& AssetData)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
}

FReply SCPUnusedAssetsReport::OnDeleteClicked()
{
	DeleteAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnMarkAsCoreClicked()
{
	MarkAssetsAsCore(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPUnusedAssetsReport::OnExcludeFromPackageClicked()
{
	ExcludeAssetsFromPackage(GetAssetsForAction());
	return FReply::Handled();
}

void SCPUnusedAssetsReport::ReferenceViewerAssets(const TArray<FAssetData> Assets)
{
	LOG_TRACE();

	TArray<FName> AssetNames;
	Algo::Transform(
		Assets,
		AssetNames,
		[](const FAssetData& AssetData)
		{
			return AssetData.PackageName;
		}
	);

	IAssetManagerEditorModule::Get().OpenReferenceViewerUI(AssetNames);
}

void SCPUnusedAssetsReport::AuditAssets(const TArray<FAssetData> Assets)
{
	LOG_TRACE();

	IAssetManagerEditorModule::Get().OpenAssetAuditUI(Assets);
}
void SCPUnusedAssetsReport::SoftRevertFiles(const TArray<FAssetData> Assets)
{
	LOG_TRACE();

	TArray<UPackage*> PackagesToUpdate;
	Algo::Transform(
		Assets,
		PackagesToUpdate,
		[](const FAssetData& Asset)
		{
			return Asset.GetPackage();
		}
	);

	ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
	const TSharedRef<FRevert> RevertOperation = ISourceControlOperation::Create<FRevert>();

	RevertOperation->SetSoftRevert(true);
	SourceControlProvider.Execute(RevertOperation, PackagesToUpdate);
}

void SCPUnusedAssetsReport::DeleteAssets(const TArray<FAssetData> Assets)
{
	LOG_TRACE();

	TArray<UObject*> ObjectsToDelete;
	Algo::Transform(
		Assets,
		ObjectsToDelete,
		[](const FAssetData& AssetData)
		{
			return AssetData.GetAsset();
		}
	);

	ObjectTools::DeleteObjects(ObjectsToDelete);
	RemoveFromList(Assets);
}

void SCPUnusedAssetsReport::ExcludeAssetsFromPackage(const TArray<FAssetData> Assets)
{
	LOG_TRACE();

	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->ExcludeAssetsFromPackage(Assets);

	RemoveFromList(Assets);
}

void SCPUnusedAssetsReport::MarkAssetsAsCore(const TArray<FAssetData> Assets)
{
	LOG_TRACE();

	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->MarkAssetsAsCore(Assets);

	RemoveFromList(Assets);
}

bool SCPUnusedAssetsReport::FilterDisplayedAsset(const FAssetData& AssetData) const
{
	return !ReportAssets.Contains(AssetData);
}

void SCPUnusedAssetsReport::RemoveFromList(const TArray<FAssetData> Assets)
{
	ReportAssets.RemoveAllSwap(
		[&Assets](const FAssetData& AssetData)
		{
			return Assets.Contains(AssetData);
		}
	);

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

int64 SCPUnusedAssetsReport::GetAssetsDiskSize(const TArray<FAssetData>& Assets) const
{
	int64 TotalDiskSize = 0;
	for (const FAssetData& AssetDataReported : Assets)
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