// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPMenuWidget.h"
#include "Widgets/Layout/SSeparator.h"
#include "../Private/PluginManager.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace
{
	const float ScrollbarPaddingSize	= 16.0f;
	const FMargin LeftRowPadding		= FMargin(0.0f, 2.5f, 2.0f, 2.5f);
	const FMargin RightRowPadding		= FMargin(3.0f, 2.5f, 2.0f, 2.5f);
}

class CalculateSpaceGainedAsyncTask : public FNonAbandonableTask
{
	TArray<FAssetData> AllAssets;
	TArray<FAssetData> AllWorlds;
public:
	CalculateSpaceGainedAsyncTask(TArray<FAssetData> InAllAssets, TArray<FAssetData> InAllWorlds) 
	{
		AllAssets = InAllAssets; 
		AllWorlds = InAllWorlds;
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(CalculateSpaceGainedAsyncTask, STATGROUP_ThreadPoolAsyncTasks); }

	/*This function is executed when we tell our task to execute*/
	void DoWork()
	{
		TArray<FAssetData> UnusedAssets = CPOperations::CheckForUnusuedAssets(AllAssets, AllWorlds);
		int64 SpaceToGain = CPOperations::GetAssetsDiskSize(UnusedAssets);

		UE_LOG(LogTemp, Warning, TEXT("DONE %lld"), SpaceToGain);
	}
};

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
				.Text(LOCTEXT("GoToDocs", "Open Documentation"))
				.ToolTipText(LOCTEXT("GoToDocsTip", "Open our documentation to get a better understand of the plugin."))
				.OnClicked(this, &SCPMenuWidget::OnGoToDocumentation)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshMemoryToGain", "Refresh Stats"))
				.ToolTipText(LOCTEXT("RefreshMemoryToGainTip", "Update the memory to gained to running a check. WARNING: This might take a while."))
				.OnClicked(this, &SCPMenuWidget::OnRefreshSpaceToGain)
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
	return SpaceToGain;
}

FReply SCPMenuWidget::OnRefreshSpaceToGain()
{
	TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
	TArray<FAssetData> AllWorlds = CPOperations::GetAllGameAssets<UWorld>();
	{
		FScopedSlowTask SlowTask(AllAssets.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
		bool bShowCancelButton = true;
		bool bAllowPIE = false;
		SlowTask.MakeDialog(bShowCancelButton, bAllowPIE);

		for (const FAssetData& AssetToLoad : AllAssets)
		{
			FString AssetName = AssetToLoad.GetFullName();
			SlowTask.EnterProgressFrame(1.f, FText::FromString(AssetName));
			AssetToLoad.GetPackage();
		}
	}

	(new FAutoDeleteAsyncTask<CalculateSpaceGainedAsyncTask>(AllAssets, AllWorlds))->StartBackgroundTask();

	//SpaceToGain = CPOperations::GetUnusuedAssetsDiskSize();
	return FReply::Handled();
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