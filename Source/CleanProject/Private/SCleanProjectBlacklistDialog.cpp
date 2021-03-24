// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.


#include "SCleanProjectBlacklistDialog.h"

#include "Interfaces/IMainFrameModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/Public/Widgets/Layout/SUniformGridPanel.h"
#include "Slate/Public/Widgets/Input/SButton.h"
#include "Slate/Public/Widgets/Text/STextBlock.h"
#include "EditorStyle/Public/EditorStyleSet.h"
#include "UnrealEd/Public/ObjectTools.h"
#include "CleanProjectSettings.h"
#include "Misc/FileHelper.h"
#include "AssetManagerEditorModule.h"
#include "IAssetRegistry.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"

#include "Framework/Commands/UIAction.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "EditorStyleSet.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SViewport.h"
#include "FileHelpers.h"
#include "ARFilter.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetRegistryModule.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "FrontendFilterBase.h"
#include "Slate/SceneViewport.h"
#include "ObjectEditorUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/BlueprintCore.h"
#include "Widgets/Input/SComboBox.h"
#include "Framework/Application/SlateApplication.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Blueprint/BlueprintSupport.h"
#include "Editor.h"
#include "ContentBrowser/Private/SAssetDialog.h"
#include "ContentBrowser/Private/SAssetPicker.h"
#include "ContentBrowser/Private/SAssetView.h"
#include "CleanProjectGameSettings.h"
#include "CleanProjectOperations.h"
#include "Widgets/Layout/SSpacer.h"
#include "CleanProjectModule.h"

#define LOCTEXT_NAMESPACE "CleanProject"

bool SCleanProjectBlacklistDialog::OpenBlacklistDialog(const TArray<FAssetData>& AssetsToBlacklist)
{
	const FVector2D DEFAULT_WINDOW_SIZE = FVector2D(400, 100);

	/** Create the window to host our package dialog widget */
	TSharedRef< SWindow > DeleteAssetsWindow = SNew(SWindow)
		.Title(LOCTEXT("CleanProject_BlacklistDialogTitle", "Clean Project Blacklist"))
		.ClientSize(DEFAULT_WINDOW_SIZE);

	/** Set the content of the window to our package dialog widget */
	TSharedRef< SCleanProjectBlacklistDialog > DeleteDialog =
		SNew(SCleanProjectBlacklistDialog, AssetsToBlacklist)
		.ParentWindow(DeleteAssetsWindow);

	DeleteAssetsWindow->SetContent(DeleteDialog);

	/** Show the package dialog window as a modal window */
	GEditor->EditorAddModalWindow(DeleteAssetsWindow);

	return DeleteDialog->DidDeleteAssets();
}

void SCleanProjectBlacklistDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	// Get info from args.
	ParentWindow = InArgs._ParentWindow;
	AssetsToBlacklist = AssetsToReport;

	// Get info from settings.
	auto Settings = GetDefault<UCleanProjectSettings>();
	
	// Prepare the display texts for the dropdowns.
	TSharedPtr<FString> AllConfigurations(new FString("All Configurations"));
	TSharedPtr<FString> AllPlatforms(new FString("All Platforms"));

	ConfigurationsDisplayTexts.Add(AllConfigurations);
	PlatformsDisplayTexts.Add(AllPlatforms);

	for (const FString& Configuration : Settings->BlacklistFiles)
	{
		TSharedPtr<FString> ConfigurationStr(new FString(Configuration));
		ConfigurationsDisplayTexts.Add(ConfigurationStr);
	}

	for (const FString& Platform : Settings->PlatformsPaths)
	{
		TSharedPtr<FString> PlatformStr(new FString(Platform));
		PlatformsDisplayTexts.Add(PlatformStr);
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CleanProject_BlacklistDialogSubtitle", "Choose how to blacklist the selected assets."))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))

			+ SUniformGridPanel::Slot(0, 0)
			[
				SAssignNew(PlatformCombobox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&PlatformsDisplayTexts)
				.InitiallySelectedItem(PlatformsDisplayTexts[0])
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
				.Content()
					[
						SNew(STextBlock).Text(this, &SCleanProjectBlacklistDialog::GetPlatformText)
					]
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SAssignNew(ConfigurationCombobox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ConfigurationsDisplayTexts)
				.InitiallySelectedItem(ConfigurationsDisplayTexts[0])
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
				.Content()
					[
						SNew(STextBlock).Text(this, &SCleanProjectBlacklistDialog::GetConfigurationText)
					]
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SAssignNew(ToggleAppendCheckbox, SCheckBox)
				.IsChecked(this, &SCleanProjectBlacklistDialog::IsAppendCheckboxChcked)
				.OnCheckStateChanged(this, &SCleanProjectBlacklistDialog::OnAppendCheckboxChecked)
				.ToolTipText(LOCTEXT("CleanProject_BlacklistAppendTip", "Ticking this checkbox will append the generated blacklist to the existing one instead of overridding it completly."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CleanProject_BlacklistAppend", "Append?"))
				]
			]
		]

		// Buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
            .Padding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			[
                SAssignNew(ToggleSkipCheckbox, SCheckBox)
                .IsChecked(this, &SCleanProjectBlacklistDialog::IsSkipCheckboxChcked)
				.OnCheckStateChanged(this, &SCleanProjectBlacklistDialog::OnSkipDialogChanged)
				.ToolTipText(LOCTEXT("CleanProject_BlacklistSkipTip", "Skip this step next time and automatically generate for all configurations."))
				[
				    SNew(STextBlock)
				    .Text(LOCTEXT("CleanProject_BlacklistSkipStep", "Skip next time."))
				]
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
                .OnClicked(this, &SCleanProjectBlacklistDialog::OnBlacklistOk)
                .Text(LOCTEXT("CleanProject_BlacklistDialogOk", "Blacklist"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
                .OnClicked(this, &SCleanProjectBlacklistDialog::OnBlacklistCancel)
                .Text(LOCTEXT("CleanProject_BlacklistDialogCancel", "Cancel"))
			]
		]
	];
}

FText SCleanProjectBlacklistDialog::GetConfigurationText() const
{
	TSharedPtr<FString> SelectedText = ConfigurationCombobox->GetSelectedItem();
	if (SelectedText.IsValid())
	{
		return FText::FromString(*SelectedText);
	}
	
	return LOCTEXT("CleanProject_BlacklistDialogComoboboxDefault", "Select option.");
}

FText SCleanProjectBlacklistDialog::GetPlatformText() const
{
	TSharedPtr<FString> SelectedText = PlatformCombobox->GetSelectedItem();
	if (SelectedText.IsValid())
	{
		return FText::FromString(*SelectedText);
	}

	return LOCTEXT("CleanProject_BlacklistDialogComoboboxDefault", "Select option.");
}

ECheckBoxState SCleanProjectBlacklistDialog::IsAppendCheckboxChcked() const
{
    auto Settings = GetDefault<UCleanProjectSettings>();
    
	if (Settings->bShouldAppendDefault)
	{
		return ECheckBoxState::Checked;
	}
	else
	{
		return ECheckBoxState::Unchecked;
	}
}

ECheckBoxState SCleanProjectBlacklistDialog::IsSkipCheckboxChcked() const
{
	auto Settings = GetDefault<UCleanProjectSettings>();

	if (Settings->bShouldSkipBlacklistDialog)
	{
		return ECheckBoxState::Checked;
	}
	else
	{
		return ECheckBoxState::Unchecked;
	}
}

FReply SCleanProjectBlacklistDialog::OnBlacklistOk()
{
	bDidDeleteAssets = true;

    auto Settings = GetDefault<UCleanProjectSettings>();
	FString SelectedPlatform = GetPlatformText().ToString();
	FString SelectedConfiguration = GetConfigurationText().ToString();

	CleanProjectOperations::GenerateBlacklist(AssetsToBlacklist, Settings->bShouldAppendDefault, SelectedPlatform, SelectedConfiguration);

	ParentWindow.Get()->RequestDestroyWindow();

	return FReply::Handled();
}

FReply SCleanProjectBlacklistDialog::OnBlacklistCancel()
{
	bDidDeleteAssets = false;

	ParentWindow.Get()->RequestDestroyWindow();

	return FReply::Handled();
}

void SCleanProjectBlacklistDialog::OnSkipDialogChanged(ECheckBoxState newState)
{
	auto Settings = GetMutableDefault<UCleanProjectSettings>();
	Settings->bShouldSkipBlacklistDialog = (newState == ECheckBoxState::Checked);
}

void SCleanProjectBlacklistDialog::OnAppendCheckboxChecked(ECheckBoxState newState)
{
    auto Settings = GetMutableDefault<UCleanProjectSettings>();
    Settings->bShouldAppendDefault = (newState == ECheckBoxState::Checked);
}

#undef LOCTEXT_NAMESPACE