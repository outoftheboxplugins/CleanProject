// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"

struct FDependReportNode;

typedef STreeView< TSharedPtr<struct FDependReportNode> > DependReportTree;

struct FDependReportNode
{
	/** The name of the tree node without the path */
	FString NodeName;
	/** If true, this node is a folder instead of a package */
	bool bIsFolder;
	/** The children of this node */
	TArray< TSharedPtr<FDependReportNode> > Children;

	/** Constructor */
	FDependReportNode();
	FDependReportNode(const FString& InNodeName, bool InIsFolder);

	/** Adds the path to the tree relative to this node, creating nodes as needed. */
	void AddPackage(const FString& PackageName);

	/** Expands this node and all its children */
	void ExpandChildrenRecursively(const TSharedRef<DependReportTree>& Treeview);

private:
	/** Helper function for AddPackage. PathElements is the tokenized path delimited by "/" */
	void AddPackage_Recursive(TArray<FString>& PathElements);
};

class SDependReportDialog : public SCompoundWidget
{
public:
	DECLARE_DELEGATE(FOnReportConfirmed)

	SLATE_BEGIN_ARGS( SDependReportDialog ){}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct( const FArguments& InArgs, const FText& InReportMessage, const TArray<FString>& InPackageNames, const FOnReportConfirmed& InOnReportConfirmed, const FOnReportConfirmed& InOnReportBlacklist);

	/** Opens the dialog in a new window */
	static void OpenDependReportDialog(const FText& ReportMessage, const TArray<FString>& PackageNames, const FOnReportConfirmed& InOnReportConfirmed, const FOnReportConfirmed& InOnReportBlacklist);

	/** Closes the dialog. */
	void CloseDialog();

private:
	/** Constructs the node tree given the list of package names */
	void ConstructNodeTree(const TArray<FString>& PackageNames);

	/** Handler to generate a row in the report tree */
	TSharedRef<ITableRow> GenerateTreeRow( TSharedPtr<FDependReportNode> TreeItem, const TSharedRef<STableViewBase>& OwnerTable );

	/** Gets the children for the specified tree item */
	void GetChildrenForTree( TSharedPtr<FDependReportNode> TreeItem, TArray< TSharedPtr<FDependReportNode> >& OutChildren );

	/** Determines which image to display next to a node */
	const FSlateBrush* GetNodeIcon(const TSharedPtr<FDependReportNode>& ReportNode) const;

	/** Handler for when "Ok" is clicked */
	FReply OkClicked();

	/** Handler for when "BlackList" is clicked */
	FReply OnBlackList();

	/** Handler for when "Cancel" is clicked */
	FReply CancelClicked();

private:
	FOnReportConfirmed OnReportConfirmed;
	FOnReportConfirmed OnReportBlackList;
	FDependReportNode DependReportRootNode;
	TSharedPtr<DependReportTree> ReportTreeView;

	/** Brushes for the different node states */
	const FSlateBrush* FolderOpenBrush;
	const FSlateBrush* FolderClosedBrush;
	const FSlateBrush* PackageBrush;
};
