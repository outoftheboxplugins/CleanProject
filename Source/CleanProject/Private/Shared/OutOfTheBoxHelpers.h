// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

namespace OutOfTheBoxHelpers
{
	FText const OutOfTheBoxCategoryText = INVTEXT("Out-of-the-Box Plugins");
	FName const OutOfTheBoxCategoryName = "Out-of-the-Box Plugins";

	TSharedRef<FWorkspaceItem> GetSharedWindowsCategory();
	FToolMenuSection& GetSharedActionsCategory();
}
