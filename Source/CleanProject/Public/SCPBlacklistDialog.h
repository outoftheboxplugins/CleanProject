// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Editor/EditorEngine.h"

class SCheckBox;

class SCPBlacklistDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCPBlacklistDialog) {}
	SLATE_ATTRIBUTE(TSharedPtr<SWindow>, ParentWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);

// Interface
public:
	// Returns true if the assets were successfully blacklisted. False for cancel.
	static bool OpenBlacklistDialog(const TArray<FAssetData>& AssetsToBlacklist);

private:
	using FStringPtr = TSharedPtr<FString>;
	using FStringComboBoxPtr = TSharedPtr<SComboBox<FStringPtr>>;

private:
    bool DidDeleteAssets() const { return bDidDeleteAssets; }

	FText GetConfigurationText() const;
	FText GetPlatformText() const;
	
	ECheckBoxState IsAppendCheckboxChecked() const;
	ECheckBoxState IsSkipCheckboxChecked() const;

	FReply OnBlacklistOk();
	FReply OnBlacklistCancel();

	void OnSkipDialogChanged(ECheckBoxState NewState) const;
	void OnAppendCheckboxChecked(ECheckBoxState NewState) const;

	void PrepareComboTexts(TArray<FStringPtr>& OptionsArray, const FString& DefaultOption, const TArray<FString>& OtherOptions) const;

private:
	TAttribute<TSharedPtr<SWindow>> ParentWindow;
	TSharedPtr<SCheckBox> ToggleAppendCheckbox;
	TSharedPtr<SCheckBox> ToggleSkipCheckbox;

	FStringComboBoxPtr ConfigurationCombobox;
	FStringComboBoxPtr PlatformCombobox;

	TArray<FStringPtr> ConfigurationsDisplayTexts;
	TArray<FStringPtr> PlatformsDisplayTexts;

	TArray<FAssetData> AssetsToBlacklist;
	bool bDidDeleteAssets = false;
};