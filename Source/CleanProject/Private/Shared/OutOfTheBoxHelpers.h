// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

namespace OutOfTheBoxHelpers
{
	FText const OutOfTheBoxCategoryText = INVTEXT("Out-of-the-Box Plugins");
	FName const OutOfTheBoxCategoryName = TEXT("Out-of-the-Box Plugins");
	/**
	 * @brief Gets (or creates if not already existing) an Out-of-the-Box Plugins category in the top bar -> Windows menu
	 * @return Out-of-the-Box Plugins category for windows registration
	 */
	TSharedRef<FWorkspaceItem> GetSharedWindowsCategory();
	/**
	 * @brief Gets (or creates if not already existing) and Out-of-the-Box Plugins category in the top bar -> Tools Menu
	 * @return Out-of-the-Box Plugins category for tools registration
	 */
	FToolMenuSection& GetSharedActionsCategory();
} // namespace OutOfTheBoxHelpers
