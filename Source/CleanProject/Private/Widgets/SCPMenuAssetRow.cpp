// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "SCPMenuAssetRow.h"

void SCPMenuAssetRow::Construct(
	const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FAssetData> InListItem)
{
	// Cache the list item so we can get information about the current Item when required
	Item = InListItem;

	FSuperRowType::Construct(InArgs, InOwnerTable);
}

TSharedRef<SWidget> SCPMenuAssetRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	// clang-format off
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
			.Text(FText::FromName(Item->PackageName))
		];
	// clang-format on

	return HorizontalBox.ToSharedRef();
}
