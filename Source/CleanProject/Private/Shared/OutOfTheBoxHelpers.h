// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

namespace OutOfTheBoxHelpers
{
	TSharedRef<FWorkspaceItem> GetPluginWorkspaceMenuCategory(FText const& PluginName);
	UToolMenu* GetSharedToolMenuCategory();
}
