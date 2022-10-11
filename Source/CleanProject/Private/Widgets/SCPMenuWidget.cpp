// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPMenuWidget.h"

#include "AssetRegistryModule.h"
#include "CPDependencyWalkerSubsystem.h"
#include "CPLog.h"
#include "CPSettings.h"
#include "CleanProjectModule.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ScopedSlowTask.h"
#include "SCPMenuAssetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPMenuWidget::Construct(const FArguments& InArgs)
{
	// clang-format off
	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		[
			SAssignNew(InuseAssetsListView, SListView<TSharedPtr<FAssetData>>)
			.ListItemsSource(&InuseAssetsList)
			.OnGenerateRow_Lambda([](TSharedPtr<FAssetData> InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPMenuAssetRow, OwnerTable, InInfo);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column("InuseAssetsList")
				.DefaultLabel(LOCTEXT("InuseAssetsList", "In use assets - whitelisted assets and explicitly cooked/packaged maps"))
			)
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(UnusedAssetsListView, SListView<TSharedPtr<FAssetData>>)
			.ListItemsSource(&UnusedAssetsList)
			.OnGenerateRow_Lambda([](TSharedPtr<FAssetData> InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPMenuAssetRow, OwnerTable, InInfo);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("UnusedAssetsList")
				.DefaultLabel(LOCTEXT("UnusedAssetsList", "Unused assets - not directly whitelisted or referenced by any actively used asset"))
			)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectUnusuedAssetsCount", "Identified unused assets"), 
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([this]()
				{
					return FText::AsNumber(UnusedAssetsList.Num());
				})))
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			SNew(SSeparator)
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
				.OnClicked(this, &SCPMenuWidget::OnRunCleanupFast)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("ComplexCleanupNow", "Complex Cleanup"))
				.ToolTipText(LOCTEXT("ComplexCleanupNowTip", "!WARNING: VERY SLOW! Loads all assets to determine unused assets in your project."))
				.OnClicked(this, &SCPMenuWidget::OnRunCleanupComplex)
				.IsEnabled(false)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenSettings", "Open Settings"))
				.ToolTipText(LOCTEXT("OpenSettingsTip", "Open plugin settings to configure the Cleanup parameters."))
				.OnClicked(this, &SCPMenuWidget::OnOpenSettings)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("GoToDocs", "Documentation"))
				.ToolTipText(LOCTEXT("GoToDocsTip", "Open our documentation to get a better understand of the plugin."))
				.OnClicked(this, &SCPMenuWidget::OnGoToDocumentation)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTip", "Refresh the stats right now."))
				.OnClicked(this, &SCPMenuWidget::OnRefreshUnused)
			]
		]
	];
	// clang-format on

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if (AssetRegistryModule.Get().IsLoadingAssets())
	{
		AssetRegistryModule.Get().OnFilesLoaded().AddSP(this, &SCPMenuWidget::OnFilesLoaded);
	}
	else
	{
		OnFilesLoaded();
	}
}

TSharedRef<SWidget> SCPMenuWidget::CreateInfoWidget(FText Title, TAttribute<FText> MetricValueAttribute)
{
	// clang-format off
	return SNew(SBorder)
		.BorderImage( FAppStyle::Get().GetBrush("Brushes.Header") )
		.Padding(FMargin(0.0f, 0.0f, 16.0f, 0.0f))
		[
			SNew(SSplitter)
			.Style(FEditorStyle::Get(), "DetailsView.Splitter")
			.PhysicalSplitterHandleSize(1.0f)
			.HitDetectionSplitterHandleSize(5.0f)
			
			+SSplitter::Slot()
			.Value(0.5f)
			.Resizable(false)
			[
				SNew(STextBlock)
				.Text(Title)
				.Justification(ETextJustify::Left)
			]
			
			+SSplitter::Slot()
			.Value(0.5f)
			.Resizable(false)
			[
				SNew(STextBlock)
				.Text(MetricValueAttribute)
				.Justification(ETextJustify::Center)
			]
		];
	// clang-format on
}

void SCPMenuWidget::OnFilesLoaded()
{
	RefreshUnusedAssets();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetAdded().AddSP(this, &SCPMenuWidget::OnAssetAdded);
	AssetRegistryModule.Get().OnAssetRemoved().AddSP(this, &SCPMenuWidget::OnAssetRemoved);
	AssetRegistryModule.Get().OnAssetRenamed().AddSP(this, &SCPMenuWidget::OnAssetRenamed);
	AssetRegistryModule.Get().OnAssetUpdatedOnDisk().AddSP(this, &SCPMenuWidget::OnAssetUpdated);
}

void SCPMenuWidget::OnAssetAdded(const FAssetData& AssetData)
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

void SCPMenuWidget::OnAssetRemoved(const FAssetData& AssetData)
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

void SCPMenuWidget::OnAssetRenamed(const FAssetData& AssetData, const FString& Name)
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

void SCPMenuWidget::OnAssetUpdated(const FAssetData& AssetData)
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

int64 SCPMenuWidget::GetUnusedAssetsCount() const
{
	return UnusedAssetsList.Num();
}

void SCPMenuWidget::RefreshUnusedAssets()
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

bool SCPMenuWidget::ShouldReactToAssetChange(const FAssetData& AssetData) const
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

FReply SCPMenuWidget::OnRunCleanupFast()
{
	UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Fast* from widget menu."));

	UCPDependencyWalkerSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Fast);
	return FReply::Handled();
}

FReply SCPMenuWidget::OnRunCleanupComplex()
{
	UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Complex* from widget menu."));

	UCPDependencyWalkerSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Complex);
	return FReply::Handled();
}

FReply SCPMenuWidget::OnRefreshUnused()
{
	RefreshUnusedAssets();
	return FReply::Handled();
}

FReply SCPMenuWidget::OnGoToDocumentation()
{
	const TSharedPtr<IPlugin> CleanProjectPlugin = IPluginManager::Get().FindPlugin("CleanProject");
	const FString DocsURL = CleanProjectPlugin->GetDescriptor().DocsURL;
	FPlatformProcess::LaunchURL(*DocsURL, nullptr, nullptr);

	return FReply::Handled();
}

FReply SCPMenuWidget::OnOpenSettings()
{
	UCPSettings::OpenSettings();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
