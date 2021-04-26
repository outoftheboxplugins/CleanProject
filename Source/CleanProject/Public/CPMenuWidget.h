// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"

/**
 * Menu Widget containing a UI interface for the developer to interact with the Clean Project.
 */

class SCPMenuWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCPMenuWidget) { }
	SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

	SCPMenuWidget();
	~SCPMenuWidget();

private:
	TSharedRef<SWidget> CreateInfoWidget(FText Title, TAttribute<FText> MetricValueAttribute);

// Resizing
private:
	void OnInfoSlotResized(float newSize) { UniformInfoSlotSize = newSize; }
	float GetInfoSlotSizeLeft() const { return UniformInfoSlotSize; }
	float GetInfoSlotSizeRight() const { return 1.0f - UniformInfoSlotSize; }

	FTimerHandle RefreshTimerHandle;
	int64 GetUnusedAssetsCount() const;
	void RefreshUnusedAssets();

// Buttons
private:
	FReply OnRunCleanupNow();
	FReply OnGoToDocumentation();

	int64 UnusedAssetsCount = 1024;
	float UniformInfoSlotSize = 0.5f;
};
