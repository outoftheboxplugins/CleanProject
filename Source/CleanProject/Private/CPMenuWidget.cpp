// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuWidget.h"

#include "CleanProjectModule.h"
#include "CPSettings.h"
#include "Widgets/Layout/SSeparator.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const float UnusedRefreshDelay = 1.5f;
	const float UnusedRefreshInterval = 7.5f;
	const float ScrollbarPaddingSize = 16.0f;

	const FName ColumnVariableName = FName("AssetName");

	const FLinearColor EnabledDependencyColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor DisabledDependencyColor = FLinearColor(0.25f, 0.25f, 0.25f, 1.0f);

	bool GetEnabledByDependencyType(const ECPAssetDependencyType AssetDependencyType)
	{
		const UCPSettings* ProjectSettings = GetDefault<UCPSettings>();
		bool bIsEnabled = false;

		switch (AssetDependencyType)
		{
		case ECPAssetDependencyType::None:
			bIsEnabled = false;
			break;
		case ECPAssetDependencyType::MapAssets:
			bIsEnabled = ProjectSettings->bCheckAllMapsReferences;
			break;
		case ECPAssetDependencyType::WhitelistAssets:
			bIsEnabled = ProjectSettings->bCheckWhitelistReferences;
			break;
		case ECPAssetDependencyType::AnyAssets:
			bIsEnabled = true;
			break;
		}

		return bIsEnabled;
	}

	bool CompareAssetDataArrays(TArray<FAssetDataPtr> LhsArray, TArray<FAssetDataPtr> RhsArray)
	{
		if (LhsArray.Num() != RhsArray.Num())
		{
			// The arrays have at least 1 different element.
			return true;
		}

		auto AlphabeticalSort = [](const FAssetDataPtr& A, const FAssetDataPtr& B)
		{
			const FName AName = *A;
			const FName BName = *B;

			return AName.Compare(BName) < 0;
		};

		LhsArray.Sort(AlphabeticalSort);
		RhsArray.Sort(AlphabeticalSort);

		const uint64 ArrayLength = LhsArray.Num();
		for (uint64 i = 0; i < ArrayLength; i++)
		{
			if (LhsArray[i] != RhsArray[i])
			{
				return true;
			}
		}

		return false;
	}

	TArray<FAssetDataPtr> TransformAssetDataArray(const TArray<FName>& AssetArray)
	{
		TArray<FAssetDataPtr> ResultArray;

		for (const auto& AssetElement : AssetArray)
		{
			ResultArray.Add(MakeShareable(new FName(AssetElement)));
		}

		return ResultArray;
	}

	TArray<FAssetDataPtr> TransformAssetDataArray(const TArray<FAssetData>& AssetArray)
	{
		TArray<FAssetDataPtr> ResultArray;

		for (const auto& AssetElement : AssetArray)
		{
			ResultArray.Add(MakeShareable(new FName(AssetElement.PackageName)));
		}

		return ResultArray;
	}
}

class SCPAssetDependencyRow final : public SMultiColumnTableRow<FAssetDataPtr>
{
public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FAssetDataPtr InListItem, ECPAssetDependencyType InAssetDependencyType);

private:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	FSlateColor GetTextColor() const;

private:
	FAssetDataPtr Item;
	ECPAssetDependencyType AssetDependencyType = ECPAssetDependencyType::None;
};

void SCPAssetDependencyRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FAssetDataPtr InListItem, ECPAssetDependencyType InAssetDependencyType)
{
	// Setting the list item since it will be used by the super constructor.
	Item = InListItem;
	AssetDependencyType = InAssetDependencyType;

	FSuperRowType::Construct(InArgs, InOwnerTable);
}

TSharedRef<SWidget> SCPAssetDependencyRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == ColumnVariableName)
	{
		return SNew(SHorizontalBox)
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
				.ColorAndOpacity(this, &SCPAssetDependencyRow::GetTextColor)
			];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not identify column based on name."));
		return SNew(STextBlock).Text(LOCTEXT("WatchUnkownColumn", "Unknown Column"));
	}
}

FSlateColor SCPAssetDependencyRow::GetTextColor() const
{
	const bool bIsEnabled = GetEnabledByDependencyType(AssetDependencyType);
	const FLinearColor TextColor = bIsEnabled ? EnabledDependencyColor : DisabledDependencyColor;
	return FSlateColor(TextColor);
}

SCPMenuWidget::~SCPMenuWidget()
{
	GEditor->GetTimerManager()->ClearTimer(RefreshTimerHandle);
}

void SCPMenuWidget::Construct(const FArguments& InArgs)
{
	const UCPSettings* ProjectSettings = GetDefault<UCPSettings>();

    ChildSlot
    [
		SNew(SVerticalBox)
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectSpaceGained", "Space gained in project"), 
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([=]()
					{
						const int64 SizeGained = ProjectSettings->GetSpaceGained();
						return (SizeGained > 0) ? FText::AsMemory(SizeGained) : LOCTEXT("UnkownSize", "UnkownSize");
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
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			SNew(SSeparator)
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(MapAssetsListView, SListView<FAssetDataPtr>)
			.ListItemsSource(&MapAssets)
			.OnGenerateRow_Lambda([](FAssetDataPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPAssetDependencyRow, OwnerTable, InInfo, ECPAssetDependencyType::MapAssets);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column(ColumnVariableName)
				.DefaultLabel(this, &SCPMenuWidget::GetMapAssetsColumnName)
			)
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(WhitelistAssetsListView, SListView<FAssetDataPtr>)
			.ListItemsSource(&WhitelistAssets)
			.OnGenerateRow_Lambda([](FAssetDataPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPAssetDependencyRow, OwnerTable, InInfo, ECPAssetDependencyType::WhitelistAssets);
				})
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column(ColumnVariableName)
				.DefaultLabel(this, &SCPMenuWidget::GetWhitelistAssetsColumnName)
			)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			SNew(SSeparator)
		]

		+SVerticalBox::Slot()
		[
			SAssignNew(DependenciesTreeView, STreeView<FAssetDataPtr>)
			.TreeItemsSource(&AssetsDependencies.TopLevelAssetsPtr)
			.OnGenerateRow_Lambda([](FAssetDataPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return SNew(SCPAssetDependencyRow, OwnerTable, InInfo, ECPAssetDependencyType::AnyAssets);
				})
			.OnGetChildren(this, &SCPMenuWidget::OnGetChildren)
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column(ColumnVariableName)
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

	GEditor->GetTimerManager()->SetTimer(RefreshTimerHandle, [=]() { RefreshUnusedAssets(); }, UnusedRefreshInterval, true, UnusedRefreshDelay);
}

TSharedRef<SWidget> SCPMenuWidget::CreateInfoWidget(FText Title, TAttribute<FText> MetricValueAttribute)
{
	return SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryMiddle"))
		.Padding(FMargin(0.0f, 0.0f, ScrollbarPaddingSize, 0.0f))
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

FText SCPMenuWidget::GetColumnNameByType(ECPAssetDependencyType AssetDependencyType) const
{
	const bool bIsEnabled = GetEnabledByDependencyType(AssetDependencyType);

	const FText EnabledText = bIsEnabled ? LOCTEXT("EnabledAsset", "Enabled") : LOCTEXT("DisabledAsset", "Disabled");

	FText AssetType;
	switch (AssetDependencyType)
	{
	case ECPAssetDependencyType::MapAssets:
		AssetType = LOCTEXT("MapAssets", "Map Assets");
		break;
	case ECPAssetDependencyType::WhitelistAssets:
		AssetType = LOCTEXT("WhitelistAssets", "Whitelist Assets");
		break;
	default:
		AssetType = LOCTEXT("AnyAssets", "Any Assets");
	}

	return FText::Format(INVTEXT("{0} - {1}"), AssetType, EnabledText);
}

void SCPMenuWidget::OnGetChildren(FAssetDataPtr InItem, TArray<FAssetDataPtr>& OutChildren)
{
	CPOperations::FChildDependency& OwnerItem = AssetsDependencies[*InItem];
	OutChildren = OwnerItem.GetChildrenAssetPtrs();
}

int64 SCPMenuWidget::GetUnusedAssetsCount() const
{
	return UnusedAssetsCount;
}

namespace
{
	template<typename T>
	void UpdateListView(TSharedPtr<SListView<FAssetDataPtr>> ListView, TArray<FAssetDataPtr>& CurrentAssets, TArray<T> NewAssetsInfo)
	{
		const TArray<FAssetDataPtr> NewAssetsArray = TransformAssetDataArray(NewAssetsInfo);
		const TArray<FAssetDataPtr> OldAssetsArray = CurrentAssets;

		CurrentAssets = NewAssetsArray;
		if (CompareAssetDataArrays(NewAssetsArray, OldAssetsArray))
		{
			ListView->RebuildList();
		}
	}
}

void SCPMenuWidget::RefreshUnusedAssets()
{
	const TArray<FAssetData> UnusedAssets = CPOperations::CheckForUnusedAssets();
	UnusedAssetsCount = UnusedAssets.Num();

	const TArray<FAssetData> NewMapAssets = CPOperations::GetAllGameAssets<UWorld>();
	UpdateListView(MapAssetsListView, MapAssets, NewMapAssets);

	const TArray<FName> NewWhitelistAssets = GetDefault<UCPSettings>()->WhitelistAssetsPaths;
	UpdateListView(WhitelistAssetsListView, WhitelistAssets, NewWhitelistAssets);

	TArray<FAssetDataPtr> AllDependencyAssets;
	AllDependencyAssets.Append(MapAssets);
	AllDependencyAssets.Append(WhitelistAssets);

	AssetsDependencies = CPOperations::GetAssetDependenciesTree(AllDependencyAssets);
	DependenciesTreeView->RebuildList();
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
	FCleanProjectModule::OpenCleanProjectSettings();

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE