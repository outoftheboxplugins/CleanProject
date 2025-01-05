// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#include "OutOfTheBoxHelpers.h"

#include <ToolMenus.h>
#include <WorkspaceMenuStructure.h>
#include <WorkspaceMenuStructureModule.h>

namespace OutOfTheBoxHelpers
{
	TSharedRef<FWorkspaceItem> GetSharedWindowsCategory()
	{
		const TSharedRef<FWorkspaceItem> Parent = WorkspaceMenu::GetMenuStructure().GetStructureRoot();
		const FText& Name = OutOfTheBoxCategoryText;
		TSharedPtr<FWorkspaceItem> FoundCategory;

		TArray<TSharedRef<FWorkspaceItem>> ExistingCategories = Parent->GetChildItems();
		const TSharedRef<FWorkspaceItem>* ExistingCategory = ExistingCategories.FindByPredicate(
			[=](TSharedRef<FWorkspaceItem> const& Category)
			{
				return Category->GetDisplayName().EqualTo(Name);
			}
		);

		if (ExistingCategory)
		{
			FoundCategory = *ExistingCategory;
		}
		else
		{
			FoundCategory = Parent->AddGroup(Name);
		}

		return FoundCategory.ToSharedRef();
	}

	FToolMenuSection& GetSharedActionsCategory()
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& SharedSection = Menu->FindOrAddSection(OutOfTheBoxCategoryName);
		SharedSection.Label = OutOfTheBoxCategoryText;

		return SharedSection;
	}
} // namespace OutOfTheBoxHelpers
