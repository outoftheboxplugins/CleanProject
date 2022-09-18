// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPMenuAssetsListView.h"

void SCPMenuAssetsListView::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FName> InListItem)
{
	// Cache the list item so we can get information about the current Item when required
	Item = InListItem;

	FSuperRowType::Construct(InArgs, InOwnerTable);
}

TSharedRef<SWidget> SCPMenuAssetsListView::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedPtr<SWidget> HorizontalBox;
	SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(6, 0, 0, 0)
		[
			SNew(SExpanderArrow, SharedThis(this)).IndentAmount(12)
		]
		
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromName(*Item))
		];

	return HorizontalBox.ToSharedRef();
}
