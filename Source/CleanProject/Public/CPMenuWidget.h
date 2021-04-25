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

private:
	TSharedRef<SWidget> CreateInfoWidget(FText Title, TAttribute<int64> SizeGainedAttribute);

private:
	int64 GetSpaceToWinNow() const;

// Resizing
private:
	void OnInfoSlotResized(float newSize) { UniformInfoSlotSize = newSize; }
	float GetInfoSlotSizeLeft() const { return UniformInfoSlotSize; }
	float GetInfoSlotSizeRight() const { return 1.0f - UniformInfoSlotSize; }

// Buttons
private:
	FReply OnRefreshSpaceToGain();
	FReply OnRunCleanupNow();
	FReply OnGoToDocumentation();

	int64 SpaceToGain = 1024;
	float UniformInfoSlotSize = 0.5f;
};
