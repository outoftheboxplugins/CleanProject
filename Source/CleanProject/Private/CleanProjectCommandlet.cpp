#include "CleanProjectCommandlet.h"

#include "CPLog.h"
#include "CPOperationsSubsystem.h"
#include "ObjectTools.h"
#include "PackageTools.h"

int32 UCleanProjectCommandlet::Main(const FString& Params)
{
	UE_LOG(LogCleanProject, Display, TEXT("Running Clean Project Commandlet with: %s."), *Params);

	const bool bLogUnused = FParse::Param(*Params, TEXT("LogUnused"));
	const bool bDelete = FParse::Param(*Params, TEXT("Delete"));

	UE_LOG(LogCleanProject, Display, TEXT("Parsed parameters LogUnused: %s Delete: %s"), *LexToString(bLogUnused),
		*LexToString(bDelete));

	TArray<FAssetData> UnusedAssets = UCPOperationsSubsystem::Get()->GetAllUnusedAssets(EScanType::Fast);
	UE_LOG(LogCleanProject, Display, TEXT("Found: %s unused assets."), *LexToString(UnusedAssets.Num()));
	if (bLogUnused)
	{
		for (const FAssetData& AssetData : UnusedAssets)
		{
			UE_LOG(LogCleanProject, Display, TEXT("- %s"), *AssetData.GetObjectPathString());
		}
	}

	TArray<UObject*> ObjectsToDelete;
	Algo::Transform(UnusedAssets, ObjectsToDelete, [](const FAssetData& AssetData) { return AssetData.GetAsset(); });

	if (UnusedAssets.Num() == 0)
	{
		UE_LOG(LogCleanProject, Display, TEXT("No unused assets found."));
	}
	else if (bDelete)
	{
		DeleteAssets(UnusedAssets);
	}
	else
	{
		UE_LOG(LogCleanProject, Display, TEXT("No deletion was performed, please use `-Delete` and `-Force` to delete assets"));
	}
	return 0;
}

void UCleanProjectCommandlet::DeleteAssets(const TArray<FAssetData>& AssetsToDelete)
{
	for (const FAssetData& Asset : AssetsToDelete)
	{
		const bool bSuccess = DeleteAsset(Asset);
		UE_CLOG(!bSuccess, LogCleanProject, Warning, TEXT("Failed to delete: %s"), *Asset.GetObjectPathString());
	}
}

bool UCleanProjectCommandlet::DeleteAsset(const FAssetData& AssetToDelete)
{
	UPackage* Package = AssetToDelete.GetPackage();
	if (!Package)
	{
		return false;
	}

	// Unload package so we can delete it
	const FString Filename = FPaths::ConvertRelativePathToFull(AssetToDelete.GetPackage()->GetLoadedPath().GetLocalFullPath());
	UE_LOG(LogTemp, Warning, TEXT("DELETING: %s"), *Filename)
	UPackageTools::UnloadPackages({Package});
	return IFileManager::Get().Delete(*Filename, true, true);
}
