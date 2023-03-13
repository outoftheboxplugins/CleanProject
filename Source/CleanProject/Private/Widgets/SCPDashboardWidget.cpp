// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPDashboardWidget.h"

#include <Engine/AssetManager.h>
#include <Interfaces/IPluginManager.h>
#include <SAssetView.h>
#include <Widgets/Input/SButton.h>

#include "CPHelpers.h"
#include "CPLog.h"
#include "CPOperationsSubsystem.h"
#include "CPSettings.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPDashboardWidget::Construct(const FArguments& InArgs)
{
	// clang-format off
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHeaderRow)
			+ SHeaderRow::Column("CoreAssetsList")
			.DefaultLabel(LOCTEXT("CoreAssetsList", "Core assets - explicity marked assets from the settings and cooked/packaged maps"))
		]

		+ SVerticalBox::Slot()
		[
			SAssignNew(CoreAssetsView, SAssetView)
			.InitialCategoryFilter(EContentBrowserItemCategoryFilter::IncludeAssets)
			.InitialThumbnailSize(EThumbnailSize::Small)
			.InitialViewType(EAssetViewType::List)
			.ShowTypeInTileView(true)
			.ShowBottomToolbar(true)
			.ShowViewOptions(true)
			.CanShowClasses(false)
			.OnShouldFilterAsset_Raw(this, &SCPDashboardWidget::FilterCoreAssets)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHeaderRow)
			+ SHeaderRow::Column("UnusedAssetsList")
			.DefaultLabel(LOCTEXT("UnusedAssetsList", "Unused assets - not part of the Core assets or referenced by any of them"))
		]

		+ SVerticalBox::Slot()
		[
			SAssignNew(UnusedAssetView, SAssetView)
			.InitialCategoryFilter(EContentBrowserItemCategoryFilter::IncludeAssets)
			.InitialThumbnailSize(EThumbnailSize::Small)
			.InitialViewType(EAssetViewType::List)
			.ShowTypeInTileView(true)
			.ShowBottomToolbar(true)
			.ShowViewOptions(true)
			.CanShowClasses(false)
			.OnShouldFilterAsset_Raw(this, &SCPDashboardWidget::FilterUnusedAsset)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([this]()
				{
					return bIsIndexDirty
						       ? LOCTEXT("DashboardOutdated", "Index is NOT up to date")
						       : LOCTEXT("DashboardUpToDate", "Index is up to date");
				})))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.Visibility_Lambda([this]()
				             {
					             return bIsIndexDirty ? EVisibility::Visible : EVisibility::Hidden;
				             })
				.ToolTipText(LOCTEXT("DashboardOutdatedTip", "Your project has changed since the last automatic indexing."
				                     "Use the Refresh button to start re-indexing now or adjust refresh parameters inside the plugin settings."))
				.OnClicked(this, &SCPDashboardWidget::OnRefreshUnused)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Refresh"))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SSpacer)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("GoToDocsTip", "Open our documentation to get a better understand of the plugin."))
				.OnClicked(this, &SCPDashboardWidget::OnGoToDocumentation)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Documentation"))
				]
			]

			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("OpenSettingsTip", "Open plugin settings to configure the Cleanup parameters."))
				.OnClicked(this, &SCPDashboardWidget::OnOpenSettings)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Settings"))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("FastCleanupNow", "Fast Cleanup"))
				.ToolTipText(LOCTEXT("FastCleanupNowTip", "Uses cached data to determine unused assets in your project."))
				.OnClicked(this, &SCPDashboardWidget::OnRunCleanupFast)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("ComplexCleanupNow", "Complex Cleanup"))
				.ToolTipText(LOCTEXT("ComplexCleanupNowTip", "!WARNING: VERY SLOW! Loads all assets to determine unused assets in your project."))
				.OnClicked(this, &SCPDashboardWidget::OnRunCleanupComplex)
				.IsEnabled(false)
			]
		]
	];
	// clang-format on

	// Callback is registered here because CallOrRegister might instantly execute it and we need to assign the slate properties
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateSP(this, &SCPDashboardWidget::OnInitialScanComplete));
}

void SCPDashboardWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (ShouldUpdateIndex())
	{
		RefreshUnusedAssets();
	}
}

void SCPDashboardWidget::OnInitialScanComplete()
{
	RefreshUnusedAssets();

	IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
	AssetRegistry.OnAssetAdded().AddSP(this, &SCPDashboardWidget::OnAssetAdded);
	AssetRegistry.OnAssetRemoved().AddSP(this, &SCPDashboardWidget::OnAssetDeleted);
	AssetRegistry.OnAssetRenamed().AddSP(this, &SCPDashboardWidget::OnAssetRenamed);
	AssetRegistry.OnAssetUpdatedOnDisk().AddSP(this, &SCPDashboardWidget::OnAssetUpdated);

	GetMutableDefault<UCPSettings>()->OnSettingsChanged.AddSP(this, &SCPDashboardWidget::OnSettingsChanged);
}

void SCPDashboardWidget::OnAssetAdded(const FAssetData& AssetData)
{
	const TArray<FAssetData>& GameAssets = CPHelpers::GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexDirty = true;
}

void SCPDashboardWidget::OnAssetDeleted(const FAssetData& AssetData)
{
	const TArray<FAssetData>& GameAssets = CPHelpers::GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexDirty = true;
}

void SCPDashboardWidget::OnAssetRenamed(const FAssetData& AssetData, const FString& Name)
{
	const TArray<FAssetData>& GameAssets = CPHelpers::GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexDirty = true;
}

void SCPDashboardWidget::OnAssetUpdated(const FAssetData& AssetData)
{
	const TArray<FAssetData>& GameAssets = CPHelpers::GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexDirty = true;
}

void SCPDashboardWidget::OnSettingsChanged()
{
	bIsIndexDirty = true;
}

void SCPDashboardWidget::RefreshUnusedAssets()
{
	const UAssetManager* AssetManager = UAssetManager::GetIfValid();
	if (!AssetManager || !AssetManager->HasInitialScanCompleted())
	{
		// Asset manager is not ready yet, delaying refresh
		return;
	}

	LastRefreshTime = FDateTime::Now();
	bIsIndexDirty = false;
	CachedCoreAssets = UCPOperationsSubsystem::Get()->GetAllCoreAssets().Array();
	CoreAssetsView->RequestSlowFullListRefresh();

	CachedUnusedAssets = UCPOperationsSubsystem::Get()->GetAllUnusedAssets(EScanType::Fast);
	UnusedAssetView->RequestSlowFullListRefresh();
}

bool SCPDashboardWidget::FilterCoreAssets(const FAssetData& AssetData) const
{
	return !CachedCoreAssets.Contains(AssetData);
}

bool SCPDashboardWidget::FilterUnusedAsset(const FAssetData& AssetData) const
{
	return !CachedUnusedAssets.Contains(AssetData);
}

bool SCPDashboardWidget::ShouldUpdateIndex() const
{
	if (!GetDefault<UCPSettings>()->bAutoRefreshDashboard)
	{
		return false;
	}

	if (!bIsIndexDirty)
	{
		return false;
	}

	const FTimespan TimeSinceLastRefresh = FDateTime::Now() - LastRefreshTime;
	const double SecondsSinceLastRefresh = TimeSinceLastRefresh.GetTotalSeconds();
	const double RefreshInterval = GetDefault<UCPSettings>()->AutoRefreshInterval;
	if (SecondsSinceLastRefresh < RefreshInterval)
	{
		return false;
	}

	return true;
}

FReply SCPDashboardWidget::OnRunCleanupFast()
{
	UCPOperationsSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Fast);
	return FReply::Handled();
}

FReply SCPDashboardWidget::OnRunCleanupComplex()
{
	UCPOperationsSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Complex);
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
