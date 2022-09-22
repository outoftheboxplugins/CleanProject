// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "OutOfTheBoxHelpers.h"

#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

namespace OutOfTheBoxHelpers
{
	TSharedRef<FWorkspaceItem> GetSharedMenuCategory()
	{
		FText const OutOfTheBoxCategoryName = INVTEXT("Out-of-the-Box");
		TSharedPtr<FWorkspaceItem> OutOfTheBoxCategory;

		TArray<TSharedRef<FWorkspaceItem>> ExistingCategories = WorkspaceMenu::GetMenuStructure().GetToolsStructureRoot()->GetChildItems();
		TSharedRef<FWorkspaceItem>* ExistingOutOfTheBoxCategory = ExistingCategories.FindByPredicate([=](TSharedRef<FWorkspaceItem> const& Category){ return Category->GetDisplayName().EqualTo(OutOfTheBoxCategoryName);});

		if(ExistingOutOfTheBoxCategory)
		{
			OutOfTheBoxCategory = *ExistingOutOfTheBoxCategory;
		}
		else
		{
			OutOfTheBoxCategory = WorkspaceMenu::GetMenuStructure().GetToolsStructureRoot()->AddGroup(OutOfTheBoxCategoryName);
		}

		return OutOfTheBoxCategory.ToSharedRef();
	}
}
