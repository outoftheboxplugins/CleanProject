// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate/Public/Widgets/Views/SListView.h"

class SCleanProjectAssetDialog : public SCompoundWidget
{
private:
	using TAssetSharedPtr = TSharedPtr<FAssetData>;

public:
	SLATE_BEGIN_ARGS(SCleanProjectAssetDialog) {}
	SLATE_END_ARGS()

	/** Opens the dialog in a new window */
	static void OpenAssetDialog(const TArray<FAssetData>& AssetsToReport);

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const TArray<FAssetData>& AssetsToReport);

	/** Closes the dialog. */
	void CloseDialog();

private:
	class SAssetLine : public SMultiColumnTableRow<TAssetSharedPtr>
	{
	public:
		SLATE_BEGIN_ARGS(SAssetLine) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TAssetSharedPtr InItem, const TSharedRef<STableViewBase>& InOwnerTable);

	private:
		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);

	private:
		TAssetSharedPtr Item;
	};

	TSharedRef<ITableRow> MakeVariableTableRow(TAssetSharedPtr InInfo, const TSharedRef<STableViewBase>& OwnerTable);

private:
	FReply OnDeleteClicked();

	FReply OnAuditClicked();

	FReply OnBlacklistClicked();

	FReply OnCancelClicked();

private:
	TArray<TAssetSharedPtr> ReportAssets;

	TSharedPtr<SListView<TAssetSharedPtr>> AssetsistView;
};