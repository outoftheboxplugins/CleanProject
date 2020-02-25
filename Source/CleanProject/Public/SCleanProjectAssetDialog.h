// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SCleanProjectAssetDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCleanProjectAssetDialog) {}
	SLATE_END_ARGS()

	/** Opens the dialog in a new window */
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Closes the dialog. */
	void CloseDialog();
};