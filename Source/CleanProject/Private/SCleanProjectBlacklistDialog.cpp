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
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "ContentBrowser/Private/SAssetDialog.h"
#include "ContentBrowser/Private/SAssetPicker.h"
#include "ContentBrowser/Private/SAssetView.h"
#include "CleanProjectGameSettings.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCleanProjectBlacklistDialog::OpenBlacklistDialog(const TArray<FAssetData>& AssetsToBlacklist)
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
}

void SCleanProjectBlacklistDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ParentWindow = InArgs._ParentWindow;

	// Prepare the display texts for the dropdowns.
	TSharedPtr<FString> AllOption(new FString("All Configurations"));

	ConfigurationsDisplayTexts.Add(AllOption);
	PlatformsDisplayTexts.Add(AllOption);

	auto Settings = GetDefault<UCleanProjectSettings>();

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
			.Text(LOCTEXT("CleanProject_BlacklistDialogSubtitle", "Choose how to blaklist the selected assets."))
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
				SAssignNew(ConfigurationCombobox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ConfigurationsDisplayTexts)
				.InitiallySelectedItem(ConfigurationsDisplayTexts[0])
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
				.Content()
					[
						SNew(STextBlock).Text(this, &SCleanProjectBlacklistDialog::ComboboxGetText, ConfigurationCombobox)
					]
			]
		+ SUniformGridPanel::Slot(1, 0)
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
						SNew(STextBlock).Text(this, &SCleanProjectBlacklistDialog::ComboboxGetText, PlatformCombobox)
					]
			]
		]


		// Buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(4, 4)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
			
			+SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCleanProjectBlacklistDialog::OnBlacklistOk)
				.Text(LOCTEXT("CleanProject_BlacklistDialogOk", "Blacklist"))
			]
			+SUniformGridPanel::Slot(1, 0)
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

FText SCleanProjectBlacklistDialog::ComboboxGetText(FStringComboBoxPtr ComboBoxPtr) const
{
	if (ComboBoxPtr)
	{
		TSharedPtr<FString> SelectedText = ComboBoxPtr->GetSelectedItem();
		if (SelectedText.IsValid())
		{
			return FText::FromString(*SelectedText);
		}
	}
	
	return LOCTEXT("CleanProject_BlacklistDialogComoboboxDefault", "Select option.");
}

FReply SCleanProjectBlacklistDialog::OnBlacklistOk()
{
	ParentWindow.Get()->RequestDestroyWindow();

	return FReply::Handled();
}


FReply SCleanProjectBlacklistDialog::OnBlacklistCancel()
{
	ParentWindow.Get()->RequestDestroyWindow();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE