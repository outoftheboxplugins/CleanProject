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
}	 // namespace CPOperations

#undef LOCTEXT_NAMESPACE
