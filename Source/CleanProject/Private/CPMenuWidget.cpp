// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuWidget.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const float ScrollbarPaddingSize	= 16.0f;
	const FMargin LeftRowPadding		= FMargin(0.0f, 2.5f, 2.0f, 2.5f);
	const FMargin RightRowPadding		= FMargin(3.0f, 2.5f, 2.0f, 2.5f);
}

void SCPMenuWidget::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
		SNew(SVerticalBox)
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectSpaceGained", "Space gained in project"), 
			TAttribute<int64>::Create(TAttribute<int64>::FGetter::CreateLambda([=]()
					{
						return GetDefault<UCPProjectSettings>()->GetSpaceGained();
					})))
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateInfoWidget(LOCTEXT("ProjectSpaceToGain", "Run the Cleanup now to save"), TAttribute<int64>(this, &SCPMenuWidget::GetSpaceToWinNow))
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
				.Text(LOCTEXT("RunCleanupNow", "Run Cleanup"))
				.ToolTipText(LOCTEXT("RunCleanupNowTip", "Start the Cleanup-Unused-Assets check now."))
				.OnClicked(this, &SCPMenuWidget::OnRunCleanupNow)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("RunCleanupNow", "Run Cleanup"))
				.ToolTipText(LOCTEXT("RunCleanupNowTip", "Start the Cleanup-Unused-Assets check now."))
				.OnClicked(this, &SCPMenuWidget::OnRunCleanupNow)
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

TSharedRef<SWidget> SCPMenuWidget::CreateInfoWidget(FText Title, TAttribute<int64> SizeGainedAttribute)
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
				.Text_Lambda([SizeGainedAttribute]
					{ 
						const int64 SizeGained = SizeGainedAttribute.Get();
						return (SizeGained > 0) ? FText::AsMemory(SizeGained) : LOCTEXT("UnkownSize", "UnkownSize");
					})
				.Justification(ETextJustify::Center)
			]
		];
}

int64 SCPMenuWidget::GetSpaceToWinNow() const
{
	return CPOperations::GetUnusuedAssetsDiskSize();
}

FReply SCPMenuWidget::OnRunCleanupNow()
{


	return FReply::Handled();
}

FReply SCPMenuWidget::OnGoToDocumentation()
{


	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE