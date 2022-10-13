// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPDashboardWidget.h"

#include "AssetRegistryModule.h"
#include "CPDependencyWalkerSubsystem.h"
#include "CPLog.h"
#include "CPSettings.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ScopedSlowTask.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPDashboardAssetRow::Construct(
	const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FCPAssetPtr InListItem)
{
	// Cache the list item so we can get information about the current Item when required
	Item = InListItem;

	FSuperRowType::Construct(InArgs, InOwnerTable);
}

TSharedRef<SWidget> SCPDashboardAssetRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	// clang-format off
	TSharedPtr<SWidget> HorizontalBox;
	SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(FText::FromName(Item->PackageName))
		];
	// clang-format on

	return HorizontalBox.ToSharedRef();
}

void SCPDashboardWidget::Construct(const FArguments& InArgs)
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfValid())
	{
		AssetManager->CallOrRegister_OnCompletedInitialScan(
			FSimpleMulticastDelegate::FDelegate::CreateSP(this, &SCPDashboardWidget::OnInitialScanComplete));
	}
	else
	{
		UE_LOG(LogCleanProject, Warning, TEXT("There is no AssetManager! Initial refresh will be skipped"));
	}

	// clang-format off
	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		[
			SAssignNew(InuseAssetsListView, SListView<FCPAssetPtr>)
			.ListItemsSource(&InuseAssetsList)
			.OnGenerateRow_Lambda([](FCPAssetPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPDashboardAssetRow, OwnerTable, InInfo);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column("InuseAssetsList")
				.DefaultLabel(LOCTEXT("InuseAssetsList", "In use assets - whitelisted assets and explicitly cooked/packaged maps"))
			)
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(UnusedAssetsListView, SListView<FCPAssetPtr>)
			.ListItemsSource(&UnusedAssetsList)
			.OnGenerateRow_Lambda([](FCPAssetPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPDashboardAssetRow, OwnerTable, InInfo);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("UnusedAssetsList")
				.DefaultLabel(LOCTEXT("UnusedAssetsList", "Unused assets - not directly whitelisted or referenced by any actively used asset"))
			)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([this]()
				{
					return FText::Format(LOCTEXT("ProjectUnusuedAssetsCount", "Identified unused assets: {0}"), UnusedAssetsList.Num());
				})))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FEditorStyle::Get().GetBrush("Icons.Warning"))
				.Visibility_Lambda([this]()
				{
					return bIsIndexOutdated ? EVisibility::Visible : EVisibility::Hidden;
				})
				.ToolTipText(LOCTEXT("ProjectUnusuedAssetsOutdated", "Your project has changed since the last automatic indexing."
					"Use the Refresh button to start re-indexing now or adjust refresh parameters inside the plugin settings."))
			]
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("FastCleanupNow", "Fast Cleanup"))
				.ToolTipText(LOCTEXT("FastCleanupNowTip", "Uses cached data to determine unused assets in your project."))
				.OnClicked(this, &SCPDashboardWidget::OnRunCleanupFast)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("ComplexCleanupNow", "Complex Cleanup"))
				.ToolTipText(LOCTEXT("ComplexCleanupNowTip", "!WARNING: VERY SLOW! Loads all assets to determine unused assets in your project."))
				.OnClicked(this, &SCPDashboardWidget::OnRunCleanupComplex)
				.IsEnabled(false)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenSettings", "Open Settings"))
				.ToolTipText(LOCTEXT("OpenSettingsTip", "Open plugin settings to configure the Cleanup parameters."))
				.OnClicked(this, &SCPDashboardWidget::OnOpenSettings)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("GoToDocs", "Documentation"))
				.ToolTipText(LOCTEXT("GoToDocsTip", "Open our documentation to get a better understand of the plugin."))
				.OnClicked(this, &SCPDashboardWidget::OnGoToDocumentation)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTip", "Refresh the stats right now."))
				.OnClicked(this, &SCPDashboardWidget::OnRefreshUnused)
			]
		]
	];
	// clang-format on
}

void SCPDashboardWidget::OnInitialScanComplete()
{
	RefreshUnusedAssets();

	IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
	AssetRegistry.OnAssetAdded().AddSP(this, &SCPDashboardWidget::OnAssetAdded);
	AssetRegistry.OnAssetRemoved().AddSP(this, &SCPDashboardWidget::OnAssetRemoved);
	AssetRegistry.OnAssetRenamed().AddSP(this, &SCPDashboardWidget::OnAssetRenamed);
	AssetRegistry.OnAssetUpdatedOnDisk().AddSP(this, &SCPDashboardWidget::OnAssetUpdated);
}

void SCPDashboardWidget::OnAssetAdded(const FAssetData& AssetData)
{
	if (ShouldReactToAssetChange(AssetData))
	{
		RefreshUnusedAssets();
	}
	else
	{
		bIsIndexOutdated = true;
	}
}

void SCPDashboardWidget::OnAssetRemoved(const FAssetData& AssetData)
{
	if (ShouldReactToAssetChange(AssetData))
	{
		RefreshUnusedAssets();
	}
	else
	{
		bIsIndexOutdated = true;
	}
}

void SCPDashboardWidget::OnAssetRenamed(const FAssetData& AssetData, const FString& Name)
{
	if (ShouldReactToAssetChange(AssetData))
	{
		RefreshUnusedAssets();
	}
	else
	{
		bIsIndexOutdated = true;
	}
}

void SCPDashboardWidget::OnAssetUpdated(const FAssetData& AssetData)
{
	if (ShouldReactToAssetChange(AssetData))
	{
		RefreshUnusedAssets();
	}
	else
	{
		bIsIndexOutdated = true;
	}
}

void SCPDashboardWidget::RefreshUnusedAssets()
{
	LastRefreshTime = FDateTime::Now();
	bIsIndexOutdated = false;

	const TArray<FAssetData> UnusedAssets = UCPDependencyWalkerSubsystem::Get()->GetAllUnusedAssets(EScanType::Fast);
	UnusedAssetsList.Empty(UnusedAssets.Num());
	Algo::Transform(UnusedAssets, UnusedAssetsList, [](const FAssetData& AssetData) { return MakeShared<FAssetData>(AssetData); });
	UnusedAssetsListView->RebuildList();

	const TArray<FAssetData> InuseAssets = UCPDependencyWalkerSubsystem::Get()->GetWhitelistedAssets().Array();
	InuseAssetsList.Empty(InuseAssets.Num());
	Algo::Transform(InuseAssets, InuseAssetsList, [](const FAssetData& AssetData) { return MakeShared<FAssetData>(AssetData); });
	InuseAssetsListView->RebuildList();
}

bool SCPDashboardWidget::ShouldReactToAssetChange(const FAssetData& AssetData) const
{
	const bool bIgnoreAssetUpdates = !GetDefault<UCPSettings>()->bAutoRefreshDashboard;
	if (bIgnoreAssetUpdates)
	{
		UE_LOG(LogCleanProject, Verbose, TEXT("Ignoring Asset change: %s based on plugin settings."), *AssetData.GetFullName())
		return false;
	}

	const TArray<FAssetData>& GameAssets = UCPDependencyWalkerSubsystem::Get()->GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);

	if (!bIsGameAsset)
	{
		UE_LOG(LogCleanProject, Verbose, TEXT("Ignoring Asset change: %s since it's not part of game assets."),
			*AssetData.GetFullName())
		return false;
	}

	const FTimespan TimeSinceLastRefresh = FDateTime::Now() - LastRefreshTime;
	const double SecondsSinceLastRefresh = TimeSinceLastRefresh.GetTotalSeconds();
	const double RefreshInterval = GetDefault<UCPSettings>()->AutoRefreshInterval;
	if (SecondsSinceLastRefresh < RefreshInterval)
	{
		UE_LOG(LogCleanProject, Verbose,
			TEXT("Skipping Unused assets refresh, only %s seconds passed, lower than refresh interval (%s seconds)"),
			*LexToString(SecondsSinceLastRefresh), *LexToString(RefreshInterval))
		return false;
	}

	return true;
}

FReply SCPDashboardWidget::OnRunCleanupFast()
{
	UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Fast* from widget menu."));

	UCPDependencyWalkerSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Fast);
	return FReply::Handled();
}

FReply SCPDashboardWidget::OnRunCleanupComplex()
{
	UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Complex* from widget menu."));

	UCPDependencyWalkerSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Complex);
	return FReply::Handled();
}

FReply SCPDashboardWidget::OnRefreshUnused()
{
	RefreshUnusedAssets();
	return FReply::Handled();
}

FReply SCPDashboardWidget::OnGoToDocumentation()
{
	const TSharedPtr<IPlugin> CleanProjectPlugin = IPluginManager::Get().FindPlugin("CleanProject");
	const FString DocsURL = CleanProjectPlugin->GetDescriptor().DocsURL;
	FPlatformProcess::LaunchURL(*DocsURL, nullptr, nullptr);

	return FReply::Handled();
}

FReply SCPDashboardWidget::OnOpenSettings()
{
	UCPSettings::OpenSettings();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
