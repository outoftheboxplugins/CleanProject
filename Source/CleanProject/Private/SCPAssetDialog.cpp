// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#define LOCTEXT_NAMESPACE "CleanProject"

void SCPAssetDialog::Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport)
{
	ReportAssets = AssetsToReport;

	int64 TotalDiskSize = 0;
	for (const FAssetData& AssetDataReported : ReportAssets)
	{
		TotalDiskSize += GetAssetDiskSize(AssetDataReported);
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		// Titlebar
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text( FText::Format(LOCTEXT("ReportDiskSize", "Total disk size: {0}"), FText::AsMemory(TotalDiskSize)))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReportSubtitle", "The following assets were found unused:"))
			.TextStyle(FEditorStyle::Get(), "PackageMigration.DialogTitle")
		]

		// Tree of packages in the report
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				CreateAssetPickerWidget()
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
				.OnClicked(this, &SCPAssetDialog::OnDeleteClicked)
				.Text(LOCTEXT("DeleteButton", "Delete"))
			]
			+SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPAssetDialog::OnAuditClicked)
				.Text(LOCTEXT("AuditButton", "More Info"))
			]
			+SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPAssetDialog::OnBlacklistClicked)
				.Text(LOCTEXT("BlacklistButton", "Blacklist"))
			]
			+SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCPAssetDialog::OnCancelClicked)
				.Text(LOCTEXT("CancelButton", "Cancel"))
			]
		]
	];
}

//////////////////////////////////////////////////////////////////////////
// Window commands

void SCPAssetDialog::OpenAssetDialog(const TArray<FAssetData>& AssetsToReport)
{
	TSharedRef<SWindow> ReportWindow = SNew(SWindow)
		.Title(LOCTEXT("AssetDialogTitle", "Clean Project Analyzer"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SCPAssetDialog, AssetsToReport)
		];

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	if (MainFrameModule.GetParentWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(ReportWindow, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(ReportWindow);
	}
}

void SCPAssetDialog::CloseAssetDialog()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());

	if (Window.IsValid())
	{
		Window->RequestDestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////////
// Slate Delegates

TSharedPtr<SWidget> SCPAssetDialog::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/ true, NULL);
	
	MenuBuilder.BeginSection(TEXT("ReportContextMenu"),
		LOCTEXT("ReportConextMenuCategory", "Cleanup actions"));
	
	MenuBuilder.AddMenuEntry(
		LOCTEXT("RemoveAction", "Remove"),
		LOCTEXT("RemoveActionTooltip", "Remove selected assets from the report, so they won't get deleted."),
		FSlateIcon(),
		FUIAction( FExecuteAction::CreateSP(this, &SCPAssetDialog::RemoveFromList, SelectedAssets) ));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AuditAction", "Audit"),
		LOCTEXT("AuditActionTooltip", "Get more information about the selected assets."),
		FSlateIcon(),
		FUIAction( FExecuteAction::CreateSP(this, &SCPAssetDialog::AuditAssets, SelectedAssets) ));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("BlacklistAction", "Blacklist"),
		LOCTEXT("BlacklistActionTooltip", "Blacklist only selected assets and remove from report."),
		FSlateIcon(),
		FUIAction( FExecuteAction::CreateSP(this, &SCPAssetDialog::BlackListAssets, SelectedAssets) ));
	
	MenuBuilder.AddMenuEntry(
		LOCTEXT("WhitelistAction", "Whitelist"),
		LOCTEXT("WhitelistActionTooltip", "Whitelist only selected assets and remove from report."),
		FSlateIcon(),
		FUIAction( FExecuteAction::CreateSP(this, &SCPAssetDialog::WhiteListAssets, SelectedAssets) ));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("DeleteAction", "Delete"),
		LOCTEXT("DeleteActionTooltip", "Delete only selected assets."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.Delete"),
		FUIAction( FExecuteAction::CreateSP(this, &SCPAssetDialog::DeleteAssets, SelectedAssets) ));

	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void SCPAssetDialog::OnRequestOpenAsset(const FAssetData& AssetData) const
{
	TArray<FName> AssetNames;
	AssetNames.Add(AssetData.PackageName);

    IAssetManagerEditorModule& ManagerEditorModule = IAssetManagerEditorModule::Get();
	ManagerEditorModule.OpenReferenceViewerUI(AssetNames);
}

//////////////////////////////////////////////////////////////////////////
// Custom Report column

int64 SCPAssetDialog::GetAssetDiskSize(const FAssetData& Asset) const
{
	const FName PackageName = FName(*Asset.GetPackage()->GetName());
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if (const FAssetPackageData* PackageData = AssetRegistryModule.Get().GetAssetPackageData(PackageName))
	{
		return PackageData->DiskSize;
	}
	return -1;
}

FString SCPAssetDialog::GetDiskSizeData(FAssetData& AssetData, FName ColumnName) const
{
	const int64 DiskSize = GetAssetDiskSize(AssetData);
	return (DiskSize > 0) ? LexToString(DiskSize) : FString("Invalid");
}

FText SCPAssetDialog::GetDiskSizeDisplayText(FAssetData& AssetData, FName ColumnName) const
{
	const int64 DiskSize = GetAssetDiskSize(AssetData);
	return (DiskSize > 0) ? FText::AsMemory(DiskSize) : LOCTEXT("UnkownSize", "UnkownSize");
}

//////////////////////////////////////////////////////////////////////////
// Buttons Actions

FReply SCPAssetDialog::OnDeleteClicked()
{
	DeleteAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPAssetDialog::OnAuditClicked()
{
	AuditAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPAssetDialog::OnBlacklistClicked()
{
	BlackListAssets(GetAssetsForAction());
	return FReply::Handled();
}

FReply SCPAssetDialog::OnCancelClicked()
{
	CloseAssetDialog();
	return FReply::Handled();
}

//////////////////////////////////////////////////////////////////////////
// Functionality

void SCPAssetDialog::DeleteAssets(const TArray<FAssetData> AssetsToDelete)
{
	LOG_TRACE();

	TArray<UObject*> ObjectsToDelete;
	for(const FAssetData& AssetData : AssetsToDelete)
	{
		ObjectsToDelete.Add(AssetData.GetAsset());
	}

	ObjectTools::DeleteObjects(ObjectsToDelete);
	RemoveFromList(AssetsToDelete);
}

void SCPAssetDialog::AuditAssets(const TArray<FAssetData> AssetsToAudit)
{
	LOG_TRACE();

	if (FModuleManager::Get().ModuleExists(TEXT("AssetManagerEditor")))
	{
		TArray<FName> AssetNames;

		for(const FAssetData& AssetData: AssetsToAudit)
		{
			AssetNames.Add(AssetData.PackageName);
		}

		IAssetManagerEditorModule& Module = FModuleManager::LoadModuleChecked<IAssetManagerEditorModule>("AssetManagerEditor");
		Module.OpenAssetAuditUI(AssetNames);
	}
}

void SCPAssetDialog::BlackListAssets(const TArray<FAssetData> AssetsToBlacklist)
{
	LOG_TRACE();

	bool bRemoveAssets = true;
	auto Settings = GetDefault<UCPEditorSettings>();
	if (Settings->bShouldSkipBlacklistDialog)
	{
		CPOperations::GenerateBlacklist(AssetsToBlacklist, Settings->bShouldAppendDefault);
	}
	else
	{
		bRemoveAssets = SCleanProjectBlacklistDialog::OpenBlacklistDialog(AssetsToBlacklist);
	}

	if (bRemoveAssets)
	{
		RemoveFromList(AssetsToBlacklist);
	}
}

void SCPAssetDialog::WhiteListAssets(const TArray<FAssetData> AssetsToWhitelist)
{
	LOG_TRACE();

	auto Settings = GetMutableDefault<UCPProjectSettings>();
	Settings->WhitelistAssets(AssetsToWhitelist);

	RemoveFromList(AssetsToWhitelist);
}

void SCPAssetDialog::RemoveFromList(const TArray<FAssetData> AssetsToRemove)
{
	LOG_TRACE();

	ReportAssets.RemoveAllSwap([&AssetsToRemove](const FAssetData& AssetData) 
		{
			return AssetsToRemove.Contains(AssetData);
		});

	if (ReportAssets.Num() == 0)
	{
		CloseAssetDialog();
	}
	else
	{
		TArray<FName> AssetObjectPaths;
		AssetObjectPaths.Reserve(AssetsToRemove.Num());

		for (const FAssetData& AssetData : AssetsToRemove)
		{
			AssetObjectPaths.Add(AssetData.ObjectPath);
		}

		ReportAssetsFilter.ObjectPaths.RemoveAllSwap([&AssetObjectPaths](const FName& objectPath)
			{
				return AssetObjectPaths.Contains(objectPath);
			});

		SetFilterDelegate.Execute(ReportAssetsFilter);
	}
}

TArray<FAssetData> SCPAssetDialog::GetAssetsForAction() const
{
	TArray<FAssetData> SelectedAssets = GetCurrentSelectionDelegate.Execute();

	if (SelectedAssets.Num())
	{
		return SelectedAssets;
	}
	else
	{
		return ReportAssets;
	}
}

TSharedRef<SWidget> SCPAssetDialog::CreateAssetPickerWidget()
{
	FAssetPickerConfig Config;
	{
		Config.InitialAssetViewType = EAssetViewType::Column;
		Config.bAddFilterUI = true;
		Config.bShowPathInColumnView = true;
		Config.bSortByPathInColumnView = true;

		// Configure response to double-click and context-menu
		Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SCPAssetDialog::OnRequestOpenAsset);
		Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SCPAssetDialog::OnGetAssetContextMenu);
		Config.SetFilterDelegates.Add(&SetFilterDelegate);
		Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);

		Config.bFocusSearchBoxWhenOpened = false;
		Config.bPreloadAssetsForContextMenu = false;

		// Hide path and type by default
		auto Settings = GetDefault<UCPEditorSettings>();
		Config.HiddenColumnNames.Append(Settings->ReportHiddenColumns);

		// Add custom columns
		Config.CustomColumns.Emplace(IAssetManagerEditorModule::DiskSizeName,
			LOCTEXT("SizeColumn", "Disk Size"),
			LOCTEXT("SizeColumnTooltip", "Size of saved file on disk for only this asset"),
			UObject::FAssetRegistryTag::TT_Numerical,
			FOnGetCustomAssetColumnData::CreateSP(this, &SCPAssetDialog::GetDiskSizeData),
			FOnGetCustomAssetColumnDisplayText::CreateSP(this, &SCPAssetDialog::GetDiskSizeDisplayText));
	}

	TArray<FName>& ReportObjectsPaths = ReportAssetsFilter.ObjectPaths;
	ReportObjectsPaths.Reserve(ReportAssets.Num());

	for (auto PackageIt = ReportAssets.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		ReportObjectsPaths.Add(PackageIt->ObjectPath);
	}

	Config.Filter = ReportAssetsFilter;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TSharedRef<SWidget> AssetPickerWidget = ContentBrowserModule.Get().CreateAssetPicker(Config);
	return AssetPickerWidget;
}

#undef LOCTEXT_NAMESPACE