// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPBlacklistDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	// Get info from args.
	ParentWindow = InArgs._ParentWindow;
	AssetsToBlacklist = AssetsToReport;

	// Get info from settings.
	auto Settings = GetDefault<UCPEditorSettings>();

	PrepareComboTexts(ConfigurationsDisplayTexts, FString("All Configurations"), Settings->BlacklistFiles);
	PrepareComboTexts(PlatformsDisplayTexts, FString("All Platforms"), Settings->PlatformsPaths);

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BlacklistDialogSubtitle", "Choose how to blacklist the selected assets."))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))

			+SUniformGridPanel::Slot(0, 0)
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
			+SUniformGridPanel::Slot(1, 0)
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
			+SUniformGridPanel::Slot(2, 0)
			[
				SAssignNew(ToggleAppendCheckbox, SCheckBox)
				.IsChecked(this, &SCPBlacklistDialog::IsAppendCheckboxChcked)
				.OnCheckStateChanged(this, &SCPBlacklistDialog::OnAppendCheckboxChecked)
				.ToolTipText(LOCTEXT("BlacklistAppendTip", "Ticking this checkbox will append the generated blacklist to the existing one instead of overridding it completly."))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BlacklistAppend", "Append?"))
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
                .IsChecked(this, &SCPBlacklistDialog::IsSkipCheckboxChcked)
				.OnCheckStateChanged(this, &SCPBlacklistDialog::OnSkipDialogChanged)
				.ToolTipText(LOCTEXT("BlacklistSkipTip", "Skip this step next time and automatically generate for all configurations."))
				[
				    SNew(STextBlock)
				    .Text(LOCTEXT("BlacklistSkipStep", "Skip next time."))
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
                .OnClicked(this, &SCPBlacklistDialog::OnBlacklistOk)
                .Text(LOCTEXT("BlacklistDialogOk", "Blacklist"))
			]
			+SHorizontalBox::Slot()
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

	TSharedRef<SWindow> DeleteAssetsWindow = SNew(SWindow)
		.Title(LOCTEXT("BlacklistDialogTitle", "Clean Project Blacklist"))
		.ClientSize(FVector2D(400, 100))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SAssignNew(DeleteDialog, SCPBlacklistDialog, AssetsToBlacklist)
			.ParentWindow(DeleteAssetsWindow)
		];

	GEditor->EditorAddModalWindow(DeleteAssetsWindow);
	
	return DeleteDialog->DidDeleteAssets();
}

FText SCPBlacklistDialog::GetConfigurationText() const
{
	FStringPtr SelectedText = ConfigurationCombobox->GetSelectedItem();
	if (SelectedText.IsValid())
	{
		return FText::FromString(*SelectedText);
	}
	
	return LOCTEXT("BlacklistDialogComoboboxDefault", "Select option.");
}

FText SCPBlacklistDialog::GetPlatformText() const
{
	FStringPtr SelectedText = PlatformCombobox->GetSelectedItem();
	if (SelectedText.IsValid())
	{
		return FText::FromString(*SelectedText);
	}

	return LOCTEXT("BlacklistDialogComoboboxDefault", "Select option.");
}

ECheckBoxState SCPBlacklistDialog::IsAppendCheckboxChcked() const
{
    auto Settings = GetDefault<UCPEditorSettings>();
    
	if (Settings->bShouldAppendDefault)
	{
		return ECheckBoxState::Checked;
	}
	else
	{
		return ECheckBoxState::Unchecked;
	}
}

ECheckBoxState SCPBlacklistDialog::IsSkipCheckboxChcked() const
{
	auto Settings = GetDefault<UCPEditorSettings>();

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

    auto Settings = GetDefault<UCPEditorSettings>();
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

void SCPBlacklistDialog::OnSkipDialogChanged(ECheckBoxState newState)
{
	auto Settings = GetMutableDefault<UCPEditorSettings>();
	Settings->bShouldSkipBlacklistDialog = (newState == ECheckBoxState::Checked);
}

void SCPBlacklistDialog::OnAppendCheckboxChecked(ECheckBoxState newState)
{
    auto Settings = GetMutableDefault<UCPEditorSettings>();
    Settings->bShouldAppendDefault = (newState == ECheckBoxState::Checked);
}

void SCPBlacklistDialog::PrepareComboTexts(TArray<FStringPtr>& OptionsArray, const FString& DefaultOption, const TArray<FString>& OtherOptions)
{
	FStringPtr DefaultOptionPtr = FStringPtr(new FString(DefaultOption));

	OptionsArray.Add(DefaultOptionPtr);

	for (const FString& Configuration : OtherOptions)
	{
		FStringPtr OptionStr = FStringPtr(new FString(Configuration));
		OptionsArray.Emplace(OptionStr);
	}
}

#undef LOCTEXT_NAMESPACE