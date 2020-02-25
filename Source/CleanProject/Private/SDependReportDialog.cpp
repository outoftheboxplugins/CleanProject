// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.


#include "SDependReportDialog.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWindow.h"
#include "Layout/WidgetPath.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Interfaces/IMainFrameModule.h"

#define LOCTEXT_NAMESPACE "DependReportDialog"

struct FCompareFDependReportNodeByName
{
	FORCEINLINE bool operator()( TSharedPtr<FDependReportNode> A, TSharedPtr<FDependReportNode> B ) const
	{
		return A->NodeName < B->NodeName;
	}
};

FDependReportNode::FDependReportNode()
	: bIsFolder(false)
{}

FDependReportNode::FDependReportNode(const FString& InNodeName, bool InIsFolder)
	: NodeName(InNodeName)
	, bIsFolder(InIsFolder)
{}

void FDependReportNode::AddPackage(const FString& PackageName)
{
	TArray<FString> PathElements;
	PackageName.ParseIntoArray(PathElements, TEXT("/"), /*InCullEmpty=*/true);

	return AddPackage_Recursive(PathElements);
}

void FDependReportNode::ExpandChildrenRecursively(const TSharedRef<DependReportTree>& Treeview)
{
	for ( auto ChildIt = Children.CreateConstIterator(); ChildIt; ++ChildIt )
	{
		Treeview->SetItemExpansion(*ChildIt, true);
		(*ChildIt)->ExpandChildrenRecursively(Treeview);
	}
}

void FDependReportNode::AddPackage_Recursive(TArray<FString>& PathElements)
{
	if ( PathElements.Num() > 0 )
	{
		// Pop the bottom element
		FString ChildNodeName = PathElements[0];
		PathElements.RemoveAt(0);

		// Try to find a child which uses this folder name
		TSharedPtr<FDependReportNode> Child;
		for ( auto ChildIt = Children.CreateConstIterator(); ChildIt; ++ChildIt )
		{
			if ( (*ChildIt)->NodeName == ChildNodeName )
			{
				Child = (*ChildIt);
				break;
			}
		}

		// If one was not found, create it
		if ( !Child.IsValid() )
		{
			const bool bIsAFolder = (PathElements.Num() > 0);
			int32 ChildIdx = Children.Add( MakeShareable(new FDependReportNode(ChildNodeName, bIsAFolder)) );
			Child = Children[ChildIdx];
			Children.Sort( FCompareFDependReportNodeByName() );
		}

		if ( ensure(Child.IsValid()) )
		{
			Child->AddPackage_Recursive(PathElements);
		}
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDependReportDialog::Construct( const FArguments& InArgs, const FText& InReportMessage, const TArray<FString>& InPackageNames, const FOnReportConfirmed& InOnReportConfirmed, const FOnReportConfirmed& InOnReportBlacklist)
{
	OnReportConfirmed = InOnReportConfirmed;
	OnReportBlackList = InOnReportBlacklist;
	FolderOpenBrush = FEditorStyle::GetBrush("ContentBrowser.AssetTreeFolderOpen");
	FolderClosedBrush = FEditorStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed");
	PackageBrush = FEditorStyle::GetBrush("ContentBrowser.ColumnViewAssetIcon");

	ConstructNodeTree(InPackageNames);
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage( FEditorStyle::GetBrush("Docking.Tab.ContentAreaBrush") )
		.Padding(FMargin(4, 8, 4, 4))
		[
			SNew(SVerticalBox)

			// Report Message
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4)
			[
				SNew(STextBlock)
				.Text(InReportMessage)
				.TextStyle( FEditorStyle::Get(), "PackageMigration.DialogTitle" )
			]

			// Tree of packages in the report
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SBorder)
				.BorderImage( FEditorStyle::GetBrush("ToolPanel.GroupBorder") )
				[
					SAssignNew( ReportTreeView, DependReportTree )
					.TreeItemsSource(&DependReportRootNode.Children)
					.ItemHeight(18)
					.SelectionMode(ESelectionMode::Single)
					.OnGenerateRow( this, &SDependReportDialog::GenerateTreeRow )
					.OnGetChildren( this, &SDependReportDialog::GetChildrenForTree )
				]
			]

			// Ok/Cancel buttons
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0,4,0,0)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
				.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
				.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
				+SUniformGridPanel::Slot(0,0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding( FEditorStyle::GetMargin("StandardDialog.ContentPadding") )
					.OnClicked(this, &SDependReportDialog::OkClicked)
					.Text(LOCTEXT("DeleteButton", "Delete"))
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SDependReportDialog::OnBlackList)
					.Text(LOCTEXT("BlackListButton", "BlackList"))
				]
				+SUniformGridPanel::Slot(2,0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding( FEditorStyle::GetMargin("StandardDialog.ContentPadding") )
					.OnClicked(this, &SDependReportDialog::CancelClicked)
					.Text(LOCTEXT("CancelButton", "Cancel"))
				]
			]
		]
	];

	if ( ensure(ReportTreeView.IsValid()) )
	{
		DependReportRootNode.ExpandChildrenRecursively(ReportTreeView.ToSharedRef());
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDependReportDialog::OpenDependReportDialog(const FText& ReportMessage, const TArray<FString>& PackageNames, const FOnReportConfirmed& InOnReportConfirmed, const FOnReportConfirmed& InOnReportBlacklist)
{
	TSharedRef<SWindow> ReportWindow = SNew(SWindow)
		.Title(LOCTEXT("ReportWindowTitle", "Depend Checker Report"))
		.ClientSize( FVector2D(600, 500) )
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			SNew(SDependReportDialog, ReportMessage, PackageNames, InOnReportConfirmed, InOnReportBlacklist)
		];

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	if ( MainFrameModule.GetParentWindow().IsValid() )
	{
		FSlateApplication::Get().AddWindowAsNativeChild(ReportWindow, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(ReportWindow);
	}
}

void SDependReportDialog::CloseDialog()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());

	if ( Window.IsValid() )
	{
		Window->RequestDestroyWindow();
	}
}

TSharedRef<ITableRow> SDependReportDialog::GenerateTreeRow( TSharedPtr<FDependReportNode> TreeItem, const TSharedRef<STableViewBase>& OwnerTable )
{
	check(TreeItem.IsValid());

	const FSlateBrush* IconBrush = GetNodeIcon(TreeItem);

	return SNew( STableRow< TSharedPtr<FDependReportNode> >, OwnerTable )
		[
			// Icon
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image( IconBrush )
			]

			// Name
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(STextBlock).Text(FText::FromString(TreeItem->NodeName))
			]
		];
}

void SDependReportDialog::GetChildrenForTree( TSharedPtr<FDependReportNode> TreeItem, TArray< TSharedPtr<FDependReportNode> >& OutChildren )
{
	OutChildren = TreeItem->Children;
}

void SDependReportDialog::ConstructNodeTree(const TArray<FString>& PackageNames)
{
	for ( auto PackageIt = PackageNames.CreateConstIterator(); PackageIt; ++PackageIt )
	{
		DependReportRootNode.AddPackage(*PackageIt);
	}
}

const FSlateBrush* SDependReportDialog::GetNodeIcon(const TSharedPtr<FDependReportNode>& ReportNode) const
{
	if ( !ReportNode->bIsFolder )
	{
		return PackageBrush;
	}
	else if ( ReportTreeView->IsItemExpanded(ReportNode) )
	{
		return FolderOpenBrush;
	}
	else
	{
		return FolderClosedBrush;
	}
}

FReply SDependReportDialog::OkClicked()
{
	CloseDialog();
	OnReportConfirmed.ExecuteIfBound();

	return FReply::Handled();
}

FReply SDependReportDialog::OnBlackList()
{
	CloseDialog();
	OnReportBlackList.ExecuteIfBound();

	return FReply::Handled();
}

FReply SDependReportDialog::CancelClicked()
{
	CloseDialog();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
