// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "OutOfTheBoxHelpers.h"

#include "ToolMenus.h"
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
			FoundCategory = Parent->AddGroup(Name);
		}

		return FoundCategory.ToSharedRef();
	}
}

namespace OutOfTheBoxHelpers
{
	TSharedRef<FWorkspaceItem> GetSharedWindowsCategory()
	{
		return OutOfTheBoxHelpersInternal::FindOrAdd(WorkspaceMenu::GetMenuStructure().GetStructureRoot(), OutOfTheBoxCategoryText);
	}
	
	FToolMenuSection& GetSharedActionsCategory()
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& SharedSection = Menu->FindOrAddSection(OutOfTheBoxCategoryName);
		SharedSection.Label = OutOfTheBoxCategoryText;

		return SharedSection;
	}
}
