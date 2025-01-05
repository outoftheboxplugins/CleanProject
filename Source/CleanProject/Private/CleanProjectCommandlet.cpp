// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#include "CleanProjectCommandlet.h"

#include <ObjectTools.h>

#include "CPLog.h"
#include "CPOperationsSubsystem.h"

int32 UCleanProjectCommandlet::Main(const FString& Params)
{
	UE_LOG(LogCleanProject, Display, TEXT("Running Clean Project Commandlet with: %s"), *Params);

	const bool bLogUnused = FParse::Param(*Params, TEXT("LogUnused"));
	const bool bDelete = FParse::Param(*Params, TEXT("Delete"));
	const bool bForce = FParse::Param(*Params, TEXT("Force"));

	UE_LOG(LogCleanProject, Display, TEXT("Parsed parameters: { LogUnused: %s Delete: %s Force: %s }"), *LexToString(bLogUnused), *LexToString(bDelete), *LexToString(bForce));

	// TODO: Implement Complex cleanup as well when that becomes available
	TArray<FAssetData> UnusedAssets = UCPOperationsSubsystem::Get()->GetAllUnusedAssets(EScanType::Fast);
	UE_LOG(LogCleanProject, Display, TEXT("Found: %s unused assets"), *LexToString(UnusedAssets.Num()));
	if (bLogUnused)
	{
		for (const FAssetData& AssetData : UnusedAssets)
		{
			UE_LOG(LogCleanProject, Display, TEXT("- %s"), *AssetData.GetObjectPathString());
		}
	}

	if (UnusedAssets.Num() == 0)
	{
		UE_LOG(LogCleanProject, Display, TEXT("No unused assets found"));
	}
	else if (bDelete)
	{
		DeleteAssets(UnusedAssets, bForce);
	}
	else
	{
		UE_LOG(LogCleanProject, Display, TEXT("No deletion was performed, please use `-Delete` and `-Force` to delete assets"));
	}
	return 0;
}

void UCleanProjectCommandlet::DeleteAssets(const TArray<FAssetData>& AssetsToDelete, bool bForce)
{
	TArray<UObject*> ObjectsToDelete;
	Algo::Transform(
		AssetsToDelete,
		ObjectsToDelete,
		[](const FAssetData& AssetData)
		{
			return AssetData.GetAsset();
		}
	);

	if (bForce)
	{
		UE_LOG(LogCleanProject, Display, TEXT("Deleting assets forcefully"));
		const int32 NumDeleted = ObjectTools::ForceDeleteObjects(ObjectsToDelete, false);
		UE_LOG(LogCleanProject, Display, TEXT("Deleted: %s assets"), *LexToString(NumDeleted));
	}
	else
	{
		UE_LOG(LogCleanProject, Display, TEXT("Deleting assets"));
		const int32 NumDeleted = ObjectTools::DeleteObjects(ObjectsToDelete, false);
		UE_LOG(LogCleanProject, Display, TEXT("Deleted: %s assets"), *LexToString(NumDeleted));
	}
}
