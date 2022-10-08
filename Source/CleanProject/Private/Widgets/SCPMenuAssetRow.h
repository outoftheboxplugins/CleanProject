// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

class SCPMenuAssetRow final : public SMultiColumnTableRow<TSharedPtr<FName>>
{
public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FAssetData> InListItem);

private:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FAssetData> Item;
};
