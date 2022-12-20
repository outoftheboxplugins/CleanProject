// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include <Commandlets/Commandlet.h>

#include "CleanProjectCommandlet.generated.h"

UCLASS()
class UCleanProjectCommandlet final : public UCommandlet
{
	GENERATED_BODY()

	// Begin UCommandlet interface
	virtual int32 Main(const FString& Params) override;
	// End UCommandlet interface

	/**
	 * @brief Performs a delete operations on the input assets and optionally forces the operations
	 * @param AssetsToDelete Assets subject to being deleted
	 * @param bForce if we should perform a normal or a force deletion
	 */
	void DeleteAssets(const TArray<FAssetData>& AssetsToDelete, bool bForce);
};
