// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#include "CPOperations.h"

#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "CPDependencyWalkerSubsystem.h"
#include "Engine/Level.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace CPOperations
{
int64 GetAssetDiskSize(const FAssetData& Asset)
{
	const FName PackageName = FName(*Asset.GetPackage()->GetName());
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TOptional<FAssetPackageData> PackageData = AssetRegistryModule.Get().GetAssetPackageDataCopy(PackageName);
	if (PackageData.IsSet())
	{
		return PackageData.GetValue().DiskSize;
	}
	return -1;
}

int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList)
{
	int64 TotalDiskSize = 0;
	{
		// FScopedSlowTask SlowTask(AssetsList.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
		const bool bShowCancelButton = true;
		const bool bAllowPie = false;
		// SlowTask.MakeDialog(bShowCancelButton, bAllowPie);

		for (const FAssetData& AssetDataReported : AssetsList)
		{
			const FText CurrentAssetName = FText::FromName(AssetDataReported.PackageName);
			const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
			// SlowTask.EnterProgressFrame(1.f, CurrentAssetText);
			TotalDiskSize += GetAssetDiskSize(AssetDataReported);
		}
	}

	return TotalDiskSize;
}

FTreeAssetDependency GetAssetDependenciesTree(const TArray<FAssetDataPtr>& AssetsNameList)
{
	TArray<FName> AssetsNames;
	for (const auto& AssetNamePtr : AssetsNameList)
	{
		AssetsNames.Add(*AssetNamePtr);
	}

	return GetAssetDependenciesTree(AssetsNames);
}

FTreeAssetDependency GetAssetDependenciesTree(const TArray<FName>& AssetsNameList)
{
	FTreeAssetDependency Result;
	for (const auto& AssetName : AssetsNameList)
	{
		Result.AddTopLevelDependency(AssetName);
	}

	// FScopedSlowTask SlowTask(AssetsNameList.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
	const bool bShowCancelButton = true;
	const bool bAllowPie = false;
	// SlowTask.MakeDialog(bShowCancelButton, bAllowPie);
	TSet<FName> PackageNamesChecked;
	for (const FChildDependency& TopDependency : Result.TopLevelDependencies)
	{
		const FName DependencyName = TopDependency.GetAssetName();
		const FText CurrentAssetName = FText::FromName(DependencyName);
		const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
		// SlowTask.EnterProgressFrame(1.f, CurrentAssetText);
		if (!PackageNamesChecked.Contains(DependencyName))
		{
			PackageNamesChecked.Add(DependencyName);
			RecursiveGetDependencies(DependencyName, PackageNamesChecked, Result);
		}
	}

	return Result;
}

void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, FTreeAssetDependency& ResultTreeDependency)
{
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FName> Dependencies;
	AssetRegistryModule.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::All);
	TArray<FAssetData> Assets;
	if (AssetRegistryModule.Get().GetAssetsByPackageName(PackageName, Assets))
	{
		for (const FAssetData& AssetData : Assets)
		{
			if (AssetData.GetClass() && AssetData.GetClass()->IsChildOf<UWorld>())
			{
				// TODO: Check if this logic actually works for external actors
				TArray<FString> ExternalActorsPackages = ULevel::GetOnDiskExternalActorPackages(PackageName.ToString());
				for (const FString& ExternalActorsPackage : ExternalActorsPackages)
				{
					FName ExternalActorPackageName = FName(ExternalActorsPackage);
					ResultTreeDependency.AddDependency(PackageName, ExternalActorPackageName);
					AllDependencies.Add(ExternalActorPackageName);
					RecursiveGetDependencies(ExternalActorPackageName, AllDependencies, ResultTreeDependency);
				}
			}
		}
	}

	for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
	{
		const FName DependencyName = *DependsIt;
		const bool bIsEnginePackage = (DependencyName).ToString().StartsWith(TEXT("/Engine"));
		const bool bIsScriptPackage = (DependencyName).ToString().StartsWith(TEXT("/Script"));

		if (!AllDependencies.Contains(DependencyName) && !bIsEnginePackage && !bIsScriptPackage)
		{
			ResultTreeDependency.AddDependency(PackageName, DependencyName);
			AllDependencies.Add(DependencyName);
			RecursiveGetDependencies(DependencyName, AllDependencies, ResultTreeDependency);
		}
	}
}

bool FChildDependency::operator!=(const FChildDependency& Other) const
{
	return !GetAssetName().IsEqual(Other.GetAssetName());
}

FChildDependency INVALID_CHILD(FName("INVALID_CHILD"));

FChildDependency::FChildDependency(const FName& InAssetName)
{
	AssetName = MakeShared<FName>(InAssetName);
}

void FChildDependency::AddDependency(const FName& ChildDependency)
{
	ChildDependencies.Emplace(ChildDependency);
}

TArray<FAssetDataPtr> FChildDependency::GetChildrenAssetPtrs()
{
	TArray<FAssetDataPtr> Children;
	for (FChildDependency& Dependency : ChildDependencies)
	{
		Children.Emplace(Dependency.AssetName);
	}

	return Children;
}

void FTreeAssetDependency::AddTopLevelDependency(const FName& AssetName)
{
	FChildDependency& NewChild = TopLevelDependencies.Emplace_GetRef(AssetName);
	TopLevelAssetsPtr.Emplace(NewChild.AssetName);
}

FChildDependency& FTreeAssetDependency::GetDependencyRecursive(const FName& OwnerName, TArray<FChildDependency>& ChildrenToCheck)
{
	for (FChildDependency& ChildToCheck : ChildrenToCheck)
	{
		if (ChildToCheck.GetAssetName() == OwnerName)
		{
			return ChildToCheck;
		}
		else
		{
			FChildDependency& ChildFound = GetDependencyRecursive(OwnerName, ChildToCheck.ChildDependencies);

			if (ChildFound != INVALID_CHILD)
			{
				return ChildFound;
			}
		}
	}

	return INVALID_CHILD;
}

void FTreeAssetDependency::GatherDependencyRecursive(
	TArray<FName>& OutResult, const TArray<FChildDependency>& ChildrenToCheck) const
{
	for (const FChildDependency& ChildToCheck : ChildrenToCheck)
	{
		OutResult.Add(ChildToCheck.GetAssetName());
		GatherDependencyRecursive(OutResult, ChildToCheck.ChildDependencies);
	}
}

void FTreeAssetDependency::AddDependency(const FName& OwnerName, const FName& DependencyName)
{
	FChildDependency& OwnerDependency = (*this)[OwnerName];
	OwnerDependency.AddDependency(DependencyName);
}

TArray<FName> FTreeAssetDependency::GetDependenciesAsList() const
{
	TArray<FName> AllDependencies;
	GatherDependencyRecursive(AllDependencies, TopLevelDependencies);
	return AllDependencies;
}
}	 // namespace CPOperations

#undef LOCTEXT_NAMESPACE
