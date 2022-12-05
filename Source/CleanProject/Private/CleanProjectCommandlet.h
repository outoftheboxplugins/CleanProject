#pragma once

#include <Commandlets/Commandlet.h>

#include "CleanProjectCommandlet.generated.h"

UCLASS()
class UCleanProjectCommandlet : public UCommandlet
{
	GENERATED_BODY()

private:
	virtual int32 Main(const FString& Params) override;

	void DeleteAssets(const TArray<FAssetData>& AssetsToDelete);
	bool DeleteAsset(const FAssetData& AssetToDelete);
};
