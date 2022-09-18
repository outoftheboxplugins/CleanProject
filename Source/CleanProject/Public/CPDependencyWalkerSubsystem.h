// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CPDependencyWalkerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CLEANPROJECT_API UCPDependencyWalkerSubsystem final : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static UCPDependencyWalkerSubsystem* Get();

	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths) const;
	TSet<FName> GetWhitelistedAssets() const;

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
