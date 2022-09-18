// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

class SCPMenuAssetRow final : public SMultiColumnTableRow<TSharedPtr<FName>>
{
public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FName> InListItem);

private:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FName> Item;
};
