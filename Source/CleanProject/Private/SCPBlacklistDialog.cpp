// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "SCPBlacklistDialog.h"

#include "CPOperations.h"
#include "CPSettings.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#include "Widgets\Input\SCheckBox.h"
#include "Widgets\Input\SButton.h"
#include "Widgets\Layout\SSpacer.h"
#include "UnrealEd\Public\Editor.h"

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPBlacklistDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	// Get info from args.
	ParentWindow = InArgs._ParentWindow;
	AssetsToBlacklist = AssetsToReport;

	// Get info from settings.
	const UCPSettings* Settings = GetMutableDefault<UCPSettings>();

	PrepareComboTexts(ConfigurationsDisplayTexts, FString("All Configurations"), Settings->BlacklistFiles);
	PrepareComboTexts(PlatformsDisplayTexts, FString("All Platforms"), Settings->PlatformsPaths);

	ChildSlot
		[
			SNew(SVerticalBox) + SVerticalBox::Slot()
			.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BlacklistDialogSubtitle", "Choose how to blacklist the selected assets."))
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
			SAssignNew(PlatformCombobox, SComboBox<FStringPtr>)
			.OptionsSource(&PlatformsDisplayTexts)
		.InitiallySelectedItem(PlatformsDisplayTexts[0])
		.OnGenerateWidget_Lambda([](FStringPtr Item)
			{
				return SNew(STextBlock).Text(FText::FromString(*Item));
			})
		.Content()
				[
					SNew(STextBlock).Text(this, &SCPBlacklistDialog::GetPlatformText)
				]
		]
	+ SUniformGridPanel::Slot(1, 0)
		[
			SAssignNew(ConfigurationCombobox, SComboBox<FStringPtr>)
			.OptionsSource(&ConfigurationsDisplayTexts)
		.InitiallySelectedItem(ConfigurationsDisplayTexts[0])
		.OnGenerateWidget_Lambda([](FStringPtr Item)
			{
				return SNew(STextBlock).Text(FText::FromString(*Item));
			})
		.Content()
				[
					SNew(STextBlock).Text(this, &SCPBlacklistDialog::GetConfigurationText)
				]
		]
	+ SUniformGridPanel::Slot(2, 0)
		[
			SAssignNew(ToggleAppendCheckbox, SCheckBox)
			.IsChecked(this, &SCPBlacklistDialog::IsAppendCheckboxChecked)
		.OnCheckStateChanged(this, &SCPBlacklistDialog::OnAppendCheckboxChecked)
		.ToolTipText(LOCTEXT("BlacklistAppendTip", "Ticking this checkbox will append the generated blacklist to the existing one instead of overridding it completly."))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BlacklistAppend", "Append?"))
		]
		]
		]

	// Buttons
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
		[
			SAssignNew(ToggleSkipCheckbox, SCheckBox)
			.IsChecked(this, &SCPBlacklistDialog::IsSkipCheckboxChecked)
		.OnCheckStateChanged(this, &SCPBlacklistDialog::OnSkipDialogChanged)
		.ToolTipText(LOCTEXT("BlacklistSkipTip", "Skip this step next time and automatically generate for all configurations."))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BlacklistSkipStep", "Skip next time"))
		]
		]
	+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSpacer)
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
		.OnClicked(this, &SCPBlacklistDialog::OnBlacklistOk)
		.Text(LOCTEXT("BlacklistDialogOk", "Blacklist"))
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
		.OnClicked(this, &SCPBlacklistDialog::OnBlacklistCancel)
		.Text(LOCTEXT("BlacklistDialogCancel", "Cancel"))
		]
		]
		];
}

bool SCPBlacklistDialog::OpenBlacklistDialog(const TArray<FAssetData>& AssetsToBlacklist)
{
	TSharedPtr<SCPBlacklistDialog> DeleteDialog;

	TSharedPtr<SWindow> DeleteAssetsWindow = SNew(SWindow)
		.Title(LOCTEXT("BlacklistDialogTitle", "Clean Project Blacklist"))
		.ClientSize(FVector2D(400, 100))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	DeleteAssetsWindow->SetContent(
		SAssignNew(DeleteDialog, SCPBlacklistDialog, AssetsToBlacklist)
		.ParentWindow(DeleteAssetsWindow)
	);

	GEditor->EditorAddModalWindow(DeleteAssetsWindow.ToSharedRef());

	return DeleteDialog->DidDeleteAssets();
}

FText SCPBlacklistDialog::GetConfigurationText() const
{
	const FStringPtr SelectedText = ConfigurationCombobox->GetSelectedItem();
	if (SelectedText.IsValid())
	{
		return FText::FromString(*SelectedText);
	}

	return LOCTEXT("BlacklistDialogComoboboxDefault", "Select option.");
}

FText SCPBlacklistDialog::GetPlatformText() const
{
	const FStringPtr SelectedText = PlatformCombobox->GetSelectedItem();
	if (SelectedText.IsValid())
	{
		return FText::FromString(*SelectedText);
	}

	return LOCTEXT("BlacklistDialogComoboboxDefault", "Select option.");
}

ECheckBoxState SCPBlacklistDialog::IsAppendCheckboxChecked() const
{
	const UCPSettings* Settings = GetMutableDefault<UCPSettings>();

	if (Settings->bShouldAppendDefault)
	{
		return ECheckBoxState::Checked;
	}
	else
	{
		return ECheckBoxState::Unchecked;
	}
}

ECheckBoxState SCPBlacklistDialog::IsSkipCheckboxChecked() const
{
	const UCPSettings* Settings = GetMutableDefault<UCPSettings>();

	if (Settings->bShouldSkipBlacklistDialog)
	{
		return ECheckBoxState::Checked;
	}
	else
	{
		return ECheckBoxState::Unchecked;
	}
}

FReply SCPBlacklistDialog::OnBlacklistOk()
{
	bDidDeleteAssets = true;

	const UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	const FString SelectedPlatform = GetPlatformText().ToString();
	const FString SelectedConfiguration = GetConfigurationText().ToString();

	CPOperations::GenerateBlacklist(AssetsToBlacklist, Settings->bShouldAppendDefault, SelectedPlatform, SelectedConfiguration);

	ParentWindow.Get()->RequestDestroyWindow();

	return FReply::Handled();
}

FReply SCPBlacklistDialog::OnBlacklistCancel()
{
	bDidDeleteAssets = false;

	ParentWindow.Get()->RequestDestroyWindow();

	return FReply::Handled();
}

void SCPBlacklistDialog::OnSkipDialogChanged(ECheckBoxState NewState) const
{
	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->bShouldSkipBlacklistDialog = (NewState == ECheckBoxState::Checked);
}

void SCPBlacklistDialog::OnAppendCheckboxChecked(ECheckBoxState newState) const
{
	UCPSettings* Settings = GetMutableDefault<UCPSettings>();
	Settings->bShouldAppendDefault = (newState == ECheckBoxState::Checked);
}

void SCPBlacklistDialog::PrepareComboTexts(TArray<FStringPtr>& OptionsArray, const FString& DefaultOption, const TArray<FString>& OtherOptions) const
{
	const FStringPtr DefaultOptionPtr = MakeShared<FString>(DefaultOption);

	OptionsArray.Add(DefaultOptionPtr);

	for (const FString& Configuration : OtherOptions)
	{
		FStringPtr OptionStr = MakeShared<FString>(Configuration);
		OptionsArray.Emplace(OptionStr);
	}
}

#undef LOCTEXT_NAMESPACE