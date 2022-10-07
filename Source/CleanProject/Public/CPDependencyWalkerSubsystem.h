// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "CPDependencyWalkerSubsystem.generated.h"

enum class EScanType
{
	Fast,
	Complex
};

/**
 *
 */
UCLASS()
class CLEANPROJECT_API UCPDependencyWalkerSubsystem final : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static UCPDependencyWalkerSubsystem* Get();

	void CheckAllDependencies(EScanType ScanType);

	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths) const;
	TArray<FAssetData> GetWhitelistedAssets() const;

private:
	TArray<FAssetData> GetAllGameAssets() const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
