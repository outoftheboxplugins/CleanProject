// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuWidget.h"
#include "Widgets/Layout/SSeparator.h"
#include "../Private/PluginManager.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const float UnusedRefreshDelay = 1.5f;
	const float UnusedRefreshInterval = 7.5f;
	const float ScrollbarPaddingSize = 16.0f;

	const FName ColumnVariableName = FName("AssetName");

	const FLinearColor EnabledDepedencyColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor DisabledDepedencyColor = FLinearColor(0.25f, 0.25f, 0.25f, 1.0f);
	
	const FMargin LeftRowPadding = FMargin(0.0f, 2.5f, 2.0f, 2.5f);
	const FMargin RightRowPadding = FMargin(3.0f, 2.5f, 2.0f, 2.5f);

	bool GetEnabledByDepedencyType(ECPAssetDependencyType AssetDependencyType)
	{
		const UCPProjectSettings* ProjectSettings = GetDefault<UCPProjectSettings>();
		bool bIsEnabled = false;

		switch (AssetDependencyType)
		{
		case ECPAssetDependencyType::None:
			break;
		case ECPAssetDependencyType::MapAssets:
			bIsEnabled = ProjectSettings->bCheckAllMapsRefernece;
			break;
		case ECPAssetDependencyType::WhitelistAssets:
			bIsEnabled = ProjectSettings->bCheckWhitelistReferences;
			break;
		}

		return bIsEnabled;
	}
}

class SCPAssetDependencyRow : public SMultiColumnTableRow<FAssetDataPtr>
{
public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, FAssetDataPtr InListItem, ECPAssetDependencyType InAssetDependencyType);

private:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	FSlateColor GetTextColor() const;

private:
	FAssetDataPtr Item;
	ECPAssetDependencyType AssetDependencyType;
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
		return SNew(STextBlock)
			.Text(FText::FromName(*Item))
			.ColorAndOpacity(this, &SCPAssetDependencyRow::GetTextColor);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not identify column based on name."));
		return SNew(STextBlock).Text(LOCTEXT("WatchUnkownColumn", "Unknown Column"));
	}
}

FSlateColor SCPAssetDependencyRow::GetTextColor() const
{
	const bool bIsEnabled = GetEnabledByDepedencyType(AssetDependencyType);
	FLinearColor TextColor = bIsEnabled ? EnabledDepedencyColor : DisabledDepedencyColor;
	return FSlateColor(TextColor);
}

SCPMenuWidget::~SCPMenuWidget()
{
	GEditor->GetTimerManager()->ClearTimer(RefreshTimerHandle);
}

void SCPMenuWidget::Construct(const FArguments& InArgs)
{
	const UCPProjectSettings* ProjectSettings = GetDefault<UCPProjectSettings>();

    ChildSlot
    [
		SNew(SVerticalBox)
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectSpaceGained", "Space gained in project"), 
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([=]()
					{
						int64 SizeGained = GetDefault<UCPProjectSettings>()->GetSpaceGained();
						return (SizeGained > 0) ? FText::AsMemory(SizeGained) : LOCTEXT("UnkownSize", "UnkownSize");
					})))
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectUnusuedAssetsCount", "Identified unused assets"), 
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([=]()
				{
					int64 UsedAssets = GetUnusedAssetsCount();
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
				.OnClicked(this, &SCPMenuWidget::OnRefreshUnushed)
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

bool SCPMenuWidget::InsertUniqueAsset(TArray<FAssetDataPtr>& ListToAdd, FName NameToAdd)
{
	FAssetDataPtr* FoundAssetPtr = ListToAdd.FindByPredicate([&NameToAdd](const FAssetDataPtr& CurrentElement)
		{
			return (*CurrentElement == NameToAdd);
		});

	if (!FoundAssetPtr)
	{
		ListToAdd.Add(MakeShareable(new FName(NameToAdd)));
		return true;
	}

	return false;
}

FText SCPMenuWidget::GetColumnNameByType(ECPAssetDependencyType AssetDependencyType) const
{
	const bool bIsEnabled = GetEnabledByDepedencyType(AssetDependencyType);

	FText EnabledText = bIsEnabled ? LOCTEXT("EnabledAsset", "Enabled") : LOCTEXT("DisabledAsset", "Disabled");

	FText AssetType;
	switch (AssetDependencyType)
	{
	case ECPAssetDependencyType::MapAssets:
		AssetType = LOCTEXT("MapAssets", "Map Assets");
		break;
	case ECPAssetDependencyType::WhitelistAssets:
		AssetType = LOCTEXT("WhitelistAssets", "Whitelist Assets");
		break;
	}

	return FText::Format(INVTEXT("{0} - {1}"), AssetType, EnabledText);
}

int64 SCPMenuWidget::GetUnusedAssetsCount() const
{
	return UnusedAssetsCount;
}

void SCPMenuWidget::RefreshUnusedAssets()
{
	TArray<FAssetData> UnusedAssets = CPOperations::CheckForUnusuedAssets();
	UnusedAssetsCount = UnusedAssets.Num();

	for (const FAssetData& WorldMapAsset : CPOperations::GetAllGameAssets<UWorld>())
	{
		const bool bNeedsRefresh = InsertUniqueAsset(MapAssets, WorldMapAsset.ObjectPath);
		if (bNeedsRefresh)
		{
			MapAssetsListView->RebuildList();
		}
	}

	for (const FName& WhiteListAsset : GetDefault<UCPProjectSettings>()->WhitelistAssetsPaths)
	{
		const bool bNeedsRefresh = InsertUniqueAsset(WhitelistAssets, WhiteListAsset);
		if (bNeedsRefresh)
		{
			WhitelistAssetsListView->RebuildList();
		}
	}
}

FReply SCPMenuWidget::OnRunCleanupNow()
{
	CPOperations::CheckAllDependencies();
	return FReply::Handled();
}

FReply SCPMenuWidget::OnRefreshUnushed()
{
	RefreshUnusedAssets();
	return FReply::Handled();
}

FReply SCPMenuWidget::OnGoToDocumentation()
{
	TSharedPtr<IPlugin> CleanProjectPlugin = FPluginManager::Get().FindPlugin("CleanProject");
	FString DocsURL = CleanProjectPlugin->GetDescriptor().DocsURL;
	FPlatformProcess::LaunchURL(*DocsURL, nullptr, nullptr);

	return FReply::Handled();
}


FReply SCPMenuWidget::OnOpenSettings()
{
	FCleanProjectModule::OpenCleanProjectSettings();

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE