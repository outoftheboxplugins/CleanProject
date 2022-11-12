// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPDashboardWidget.h"

#include "CPLog.h"
#include "CPOperationsSubsystem.h"
#include "CPSettings.h"
#include "EditorAssetLibrary.h"

#include <AssetRegistryModule.h>
#include <Engine/AssetManager.h>
#include <Interfaces/IPluginManager.h>
#include <SAssetView.h>
#include <Widgets/Input/SButton.h>

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPDashboardWidget::Construct(const FArguments& InArgs)
{
	// clang-format off
	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHeaderRow)
			+SHeaderRow::Column("InuseAssetsList")
			.DefaultLabel(LOCTEXT("InuseAssetsList", "In use assets - whitelisted assets and explicitly cooked/packaged maps"))
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(InuseAssetView, SAssetView)
			.InitialCategoryFilter(EContentBrowserItemCategoryFilter::IncludeAssets)
			.InitialThumbnailSize(EThumbnailSize::Small)
			.InitialViewType(EAssetViewType::List)
			.ShowTypeInTileView(true)
			.ShowBottomToolbar(true)
			.ShowViewOptions(true)
			.OnShouldFilterAsset_Raw(this, &SCPDashboardWidget::FilterInuseAsset)
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHeaderRow)
			+ SHeaderRow::Column("UnusedAssetsList")
			.DefaultLabel(LOCTEXT("UnusedAssetsList", "Unused assets - not directly whitelisted or referenced by any actively used asset"))
		]
		
		+SVerticalBox::Slot()
		[
			SAssignNew(UnusedAssetView, SAssetView)
			.InitialCategoryFilter(EContentBrowserItemCategoryFilter::IncludeAssets)
			.InitialThumbnailSize(EThumbnailSize::Small)
			.InitialViewType(EAssetViewType::List)
			.ShowTypeInTileView(true)
			.ShowBottomToolbar(true)
			.ShowViewOptions(true)
			.OnShouldFilterAsset_Raw(this, &SCPDashboardWidget::FilterUnusedAsset)
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
					return bIsIndexOutdated ? LOCTEXT("UnusuedAssetsOutdated", "Index is NOT up to date")
											: LOCTEXT("UnusuedAssetsUpdated", "Index is up to date");
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
				.ToolTipText(LOCTEXT("UnusuedAssetsOutdatedTooltip", "Your project has changed since the last automatic indexing."
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
				.Text(LOCTEXT("GenerateBlacklist", "Generate Blacklist"))
				.ToolTipText(LOCTEXT("GenerateBlacklistTip", "Generates an updated PakFileRules based on the blacklist settings."))
				.OnClicked(this, &SCPDashboardWidget::OnGenerateBlacklist)
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

	// Callback is registered here because CallOrRegister might instantly execute it and we need to assign the slate properties
	if (const UAssetManager* AssetManager = UAssetManager::GetIfValid())
	{
		AssetManager->CallOrRegister_OnCompletedInitialScan(
			FSimpleMulticastDelegate::FDelegate::CreateSP(this, &SCPDashboardWidget::OnInitialScanComplete));
	}
	else
	{
		UE_LOG(LogCleanProject, Warning, TEXT("There is no AssetManager! Initial refresh will be skipped"));
	}
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
	AssetRegistry.OnAssetRemoved().AddSP(this, &SCPDashboardWidget::OnAssetRemoved);
	AssetRegistry.OnAssetRenamed().AddSP(this, &SCPDashboardWidget::OnAssetRenamed);
	AssetRegistry.OnAssetUpdatedOnDisk().AddSP(this, &SCPDashboardWidget::OnAssetUpdated);

	GetMutableDefault<UCPSettings>()->OnSettingsChanged.AddSP(this, &SCPDashboardWidget::OnSettingsChanged);
}

void SCPDashboardWidget::OnAssetAdded(const FAssetData& AssetData)
{
	const TArray<FAssetData>& GameAssets = UCPOperationsSubsystem::Get()->GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexOutdated = true;
}

void SCPDashboardWidget::OnAssetRemoved(const FAssetData& AssetData)
{
	const TArray<FAssetData>& GameAssets = UCPOperationsSubsystem::Get()->GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexOutdated = true;
}

void SCPDashboardWidget::OnAssetRenamed(const FAssetData& AssetData, const FString& Name)
{
	const TArray<FAssetData>& GameAssets = UCPOperationsSubsystem::Get()->GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexOutdated = true;
}

void SCPDashboardWidget::OnAssetUpdated(const FAssetData& AssetData)
{
	const TArray<FAssetData>& GameAssets = UCPOperationsSubsystem::Get()->GetAllGameAssets().Array();
	const bool bIsGameAsset = GameAssets.Contains(AssetData);
	if (!bIsGameAsset)
	{
		return;
	}

	bIsIndexOutdated = true;
}

void SCPDashboardWidget::OnSettingsChanged()
{
	bIsIndexOutdated = true;
}

void SCPDashboardWidget::RefreshUnusedAssets()
{
	LastRefreshTime = FDateTime::Now();
	bIsIndexOutdated = false;
	InuseAssets = UCPOperationsSubsystem::Get()->GetWhitelistedAssets().Array();
	InuseAssetView->RequestSlowFullListRefresh();

	UnusedAssets = UCPOperationsSubsystem::Get()->GetAllUnusedAssets(EScanType::Fast);
	UnusedAssetView->RequestSlowFullListRefresh();
}

bool SCPDashboardWidget::FilterInuseAsset(const FAssetData& AssetData) const
{
	return !InuseAssets.Contains(AssetData);
}

bool SCPDashboardWidget::FilterUnusedAsset(const FAssetData& AssetData) const
{
	return !UnusedAssets.Contains(AssetData);
}

bool SCPDashboardWidget::ShouldUpdateIndex() const
{
	const bool bIgnoreAssetUpdates = !GetDefault<UCPSettings>()->bAutoRefreshDashboard;
	if (bIgnoreAssetUpdates)
	{
		return false;
	}

	if (!bIsIndexOutdated)
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
	UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Fast* from widget menu."));

	UCPOperationsSubsystem::Get()->DeleteAllUnusedAssets(EScanType::Fast);
	return FReply::Handled();
}

FReply SCPDashboardWidget::OnRunCleanupComplex()
{
	UE_LOG(LogCleanProject, Log, TEXT("Starting *Cleanup Unused Assets Complex* from widget menu."));

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

FReply SCPDashboardWidget::OnGenerateBlacklist()
{
	const FString PakFileRulesPath = FString(FPaths::GeneratedConfigDir()) / FString(TEXT("PakFileRules.ini"));
	FConfigFile* PakFileRulesConfig = GConfig->Find(PakFileRulesPath);

	PakFileRulesConfig->SetBool(TEXT("CleanProject"), TEXT("bOverrideChunkManifest"), true);
	PakFileRulesConfig->SetBool(TEXT("CleanProject"), TEXT("bExcludeFromPaks"), true);

	TArray<FString> BlacklistedFilePaths;
	const UCPSettings* Settings = GetDefault<UCPSettings>();
	for (const FSoftObjectPath& BlacklistedAsset : Settings->BlacklistedAssets)
	{
		const FString& AssetPath = FString(TEXT("...")) + BlacklistedAsset.GetAssetPathString();
		BlacklistedFilePaths.Add(AssetPath);
	}

	PakFileRulesConfig->SetArray(TEXT("CleanProject"), TEXT("Files"), BlacklistedFilePaths);

	PakFileRulesConfig->Dirty = true;
	PakFileRulesConfig->Write(PakFileRulesPath);

	return FReply::Handled();
}

FReply SCPDashboardWidget::OnOpenSettings()
{
	UCPSettings::OpenSettings();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
