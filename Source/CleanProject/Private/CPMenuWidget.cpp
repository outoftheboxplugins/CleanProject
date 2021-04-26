// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuWidget.h"
#include "Widgets/Layout/SSeparator.h"
#include "../Private/PluginManager.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const float UnusedRefreshInterval	= 7.5f;
	const float ScrollbarPaddingSize	= 16.0f;
	const FMargin LeftRowPadding		= FMargin(0.0f, 2.5f, 2.0f, 2.5f);
	const FMargin RightRowPadding		= FMargin(3.0f, 2.5f, 2.0f, 2.5f);
}

SCPMenuWidget::SCPMenuWidget()
{
	GEditor->GetTimerManager()->SetTimer(RefreshTimerHandle, [=]() { RefreshUnusedAssets(); }, UnusedRefreshInterval, true);
}

SCPMenuWidget::~SCPMenuWidget()
{
	GEditor->GetTimerManager()->ClearTimer(RefreshTimerHandle);
}

void SCPMenuWidget::Construct(const FArguments& InArgs)
{
	RefreshUnusedAssets();

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
				.Text(LOCTEXT("GoToDocs", "Open Documentation"))
				.ToolTipText(LOCTEXT("GoToDocsTip", "Open our documentation to get a better understand of the plugin."))
				.OnClicked(this, &SCPMenuWidget::OnGoToDocumentation)
			]
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			SNew(SSeparator)
		]

		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSpacer)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.Text(LOCTEXT("GoToDocumentation", "Go to documentation"))
			.ToolTipText(LOCTEXT("GoToDocumentationTip", "Visit our impressive and comprehensive documentation to help you use the system."))
			.OnClicked(this, &SCPMenuWidget::OnGoToDocumentation)
		]
    ];
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

int64 SCPMenuWidget::GetUnusedAssetsCount() const
{
	return UnusedAssetsCount;
}

void SCPMenuWidget::RefreshUnusedAssets()
{
	TArray<FAssetData> UnusedAssets = CPOperations::CheckForUnusuedAssets();
	UnusedAssetsCount = UnusedAssets.Num();
}

FReply SCPMenuWidget::OnRunCleanupNow()
{
	CPOperations::CheckAllDependencies();
	return FReply::Handled();
}

FReply SCPMenuWidget::OnGoToDocumentation()
{
	TSharedPtr<IPlugin> CleanProjectPlugin = FPluginManager::Get().FindPlugin("CleanProject");
	FString DocsURL = CleanProjectPlugin->GetDescriptor().DocsURL;
	FPlatformProcess::LaunchURL(*DocsURL, nullptr, nullptr);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE