// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SCleanProjectBlacklistDialog : public SCompoundWidget
{
private:
	using FStringComboBoxPtr = TSharedPtr<SComboBox<TSharedPtr<FString>>>;

public:
	SLATE_BEGIN_ARGS(SCleanProjectBlacklistDialog) {}

	// The parent window hosting this dialog
	SLATE_ATTRIBUTE(TSharedPtr<SWindow>, ParentWindow)

	SLATE_END_ARGS()

// Interface
public:
	// Returns true if the assets were successfully blacklisted. False for cancel.
	static bool OpenBlacklistDialog(const TArray<FAssetData>& AssetsToBlacklist);
	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);

	FText GetConfigurationText() const;
	FText GetPlatformText() const;

	FReply OnBlacklistOk();
	FReply OnBlacklistCancel();

private:
    bool DidDeleteAssets() const { return bDidDeleteAssets; }

private:
	TAttribute<TSharedPtr<SWindow>> ParentWindow;
	FStringComboBoxPtr ConfigurationCombobox;
	FStringComboBoxPtr PlatformCombobox;

	TArray<TSharedPtr<FString>> ConfigurationsDisplayTexts;
	TArray<TSharedPtr<FString>> PlatformsDisplayTexts;

	TArray<FAssetData> AssetsToBlacklist;
	bool bDidDeleteAssets = false;
};