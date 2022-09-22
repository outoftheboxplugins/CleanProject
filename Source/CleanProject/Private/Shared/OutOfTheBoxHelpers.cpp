// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "OutOfTheBoxHelpers.h"

#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

namespace  OutOfTheBoxHelpersInternal
{
	TSharedRef<FWorkspaceItem> FindOrAdd(TSharedRef<FWorkspaceItem> Parent, FText const& Name)
	{
		TSharedPtr<FWorkspaceItem> FoundCategory;

		TArray<TSharedRef<FWorkspaceItem>> ExistingCategories = Parent->GetChildItems();
		TSharedRef<FWorkspaceItem>* ExistingCategory = ExistingCategories.FindByPredicate([=](TSharedRef<FWorkspaceItem> const& Category){ return Category->GetDisplayName().EqualTo(Name);});

		if(ExistingCategory)
		{
			FoundCategory = *ExistingCategory;
		}
		else
		{
			FoundCategory = WorkspaceMenu::GetMenuStructure().GetToolsStructureRoot()->AddGroup(Name);
		}

		return FoundCategory.ToSharedRef();
	}
}

namespace OutOfTheBoxHelpers
{
	TSharedRef<FWorkspaceItem> GetPluginWorkspaceMenuCategory(FText const& PluginName)
	{
		FText const OutOfTheBoxCategoryName = INVTEXT("Out-of-the-Box");
		TSharedRef<FWorkspaceItem> const OutOfTheBoxCategory = OutOfTheBoxHelpersInternal::FindOrAdd(WorkspaceMenu::GetMenuStructure().GetToolsStructureRoot(), OutOfTheBoxCategoryName);
		TSharedRef<FWorkspaceItem> PluginCategory = OutOfTheBoxHelpersInternal::FindOrAdd(OutOfTheBoxCategory, PluginName);

		return PluginCategory;
	}
	
	UToolMenu* GetSharedToolMenuCategory()
	{
		return nullptr;
	}
}
