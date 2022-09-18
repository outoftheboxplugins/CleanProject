// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuWidget.h"

#include "CleanProjectModule.h"
#include "CPSettings.h"

#include "AssetRegistryModule.h"
#include "Widgets/Layout/SSeparator.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ScopedSlowTask.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "CleanProject"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SCPAssetDependencyRow
void SCPAssetDependencyRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FAssetDataPtr InListItem)
{
	// Cache the list item so we can get information about the current Item when required
	Item = InListItem;

	FSuperRowType::Construct(InArgs, InOwnerTable);
}

TSharedRef<SWidget> SCPAssetDependencyRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedPtr<SWidget> HorizontalBox;
	SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(6, 0, 0, 0)
		[
			SNew(SExpanderArrow, SharedThis(this)).IndentAmount(12)
		]
		
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromName(*Item))
		];

	return HorizontalBox.ToSharedRef();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SCPMenuWidget
void SCPMenuWidget::Construct(const FArguments& InArgs)
{
	UCPSettings* ProjectSettings = GetMutableDefault<UCPSettings>();
	ProjectSettings->OnAnyPropertyChanged.AddSP(this, &SCPMenuWidget::RefreshUnusedAssets);

	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(UnusedAssetsListView, SListView<FAssetDataPtr>)
			.ListItemsSource(&UnusedAssetsList)
			.OnGenerateRow_Lambda([](FAssetDataPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPAssetDependencyRow, OwnerTable, InInfo);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("UnusedAssetsList")
				.DefaultLabel(LOCTEXT("UnusedAssetsList", "In use assets - whitelisted assets + maps to be packaged"))
			)
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectSpaceGained", "Space gained in project"), 
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([=]()
					{
						return FText::AsMemory(ProjectSettings->GetSpaceGained());
					})))
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectUnusuedAssetsCount", "Identified unused assets"), 
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([=]()
				{
					const int64 UsedAssets = GetUnusedAssetsCount();
					return FText::AsNumber(UsedAssets);
				})))
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(InuseAssetsTreeView, STreeView<FAssetDataPtr>)
			.TreeItemsSource(&InuseAssetsDependencies.TopLevelAssetsPtr)
			.OnGenerateRow_Lambda([](FAssetDataPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPAssetDependencyRow, OwnerTable, InInfo);
				})
			.OnGetChildren(this, &SCPMenuWidget::OnGetChildren)
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column("DepedenciesTreeView")
				.DefaultLabel(LOCTEXT("DepedenciesTreeView", "Depedencies Tree View"))
			)
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
				.Text(LOCTEXT("RunCleanupNow", "Run Cleanup"))
				.ToolTipText(LOCTEXT("RunCleanupNowTip", "Start the Cleanup-Unused-Assets check now."))
				.OnClicked(this, &SCPMenuWidget::OnRunCleanupNow)
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

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if(AssetRegistryModule.Get().IsLoadingAssets())
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
	return SNew(SBorder)
		.BorderImage( FAppStyle::Get().GetBrush("Brushes.Header") )
		.Padding(FMargin(0.0f, 0.0f, 16.0f, 0.0f))
		[
			SNew(SSplitter)
			.Style(FEditorStyle::Get(), "DetailsView.Splitter")
			.PhysicalSplitterHandleSize(1.0f)
			.HitDetectionSplitterHandleSize(5.0f)
			
			+SSplitter::Slot()
			.Value(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(this, &SCPMenuWidget::GetInfoSlotSizeLeft)))
			.OnSlotResized(SSplitter::FOnSlotResized::CreateSP(this, &SCPMenuWidget::OnInfoSlotResized))
			[
				SNew(STextBlock)
				.Text(Title)
				.Justification(ETextJustify::Left)
			]
			
			+SSplitter::Slot()
			.Value(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(this, &SCPMenuWidget::GetInfoSlotSizeRight)))
			[
				SNew(STextBlock)
				.Text(MetricValueAttribute)
				.Justification(ETextJustify::Center)
			]
		];
}

void SCPMenuWidget::OnGetChildren(FAssetDataPtr InItem, TArray<FAssetDataPtr>& OutChildren)
{
	CPOperations::FChildDependency& OwnerItem = InuseAssetsDependencies[*InItem];
	OutChildren = OwnerItem.GetChildrenAssetPtrs();
}

void SCPMenuWidget::OnFilesLoaded()
{
	RefreshUnusedAssets();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetAdded().AddSP(this, &SCPMenuWidget::OnAssetAdded);
	AssetRegistryModule.Get().OnAssetRemoved().AddSP(this, &SCPMenuWidget::OnAssetRemoved);
	AssetRegistryModule.Get().OnAssetRenamed().AddSP(this, &SCPMenuWidget::OnAssetRenamed);
	AssetRegistryModule.Get().OnAssetUpdated().AddSP(this, &SCPMenuWidget::OnAssetUpdated);
}

void SCPMenuWidget::OnAssetAdded(const FAssetData& AssetData)
{
	if(IsGameAsset(AssetData))
	{
		RefreshUnusedAssets();
	}
}

void SCPMenuWidget::OnAssetRemoved(const FAssetData& AssetData)
{
	if(IsGameAsset(AssetData))
	{
		RefreshUnusedAssets();
	}
}

void SCPMenuWidget::OnAssetRenamed(const FAssetData& AssetData, const FString& Name)
{
	if(IsGameAsset(AssetData))
	{
		RefreshUnusedAssets();
	}
}

void SCPMenuWidget::OnAssetUpdated(const FAssetData& AssetData)
{
	if(IsGameAsset(AssetData))
	{
		RefreshUnusedAssets();
	}
}

int64 SCPMenuWidget::GetUnusedAssetsCount() const
{
	return UnusedAssetsCount;
}

void SCPMenuWidget::RefreshUnusedAssets()
{
	const TArray<FAssetData> UnusedAssets = CPOperations::CheckForUnusedAssets();
	UnusedAssetsCount = UnusedAssets.Num();

	TArray<FAssetDataPtr> AllDependencyAssets;
	const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();
	Algo::Transform(PackagingSettings->MapsToCook, AllDependencyAssets, [](FFilePath const& File){return MakeShared<FName>(File.FilePath);});
	Algo::Transform(GetDefault<UCPSettings>()->WhitelistedAssets, AllDependencyAssets, [](FSoftObjectPath const& Object){return MakeShared<FName>(Object.GetAssetPathName());});

	{
		InuseAssetsDependencies = CPOperations::GetAssetDependenciesTree(AllDependencyAssets);
		InuseAssetsTreeView->RebuildList();
	}
}

bool SCPMenuWidget::IsGameAsset(const FAssetData& AssetData) const
{
	const TArray<FAssetData>& GameAssets = CPOperations::GetAllGameAssets();
	return GameAssets.Contains(AssetData);
}

FReply SCPMenuWidget::OnRunCleanupNow()
{
	CPOperations::CheckAllDependencies();
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
