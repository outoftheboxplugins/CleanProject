// Copyright Out-of-the-Box Plugins 2018-2019. All Rights Reserved.

#include "CleanProjectOperations.h"

#include "AssetRegistryModule.h"
#include "Core/Public/Misc/ScopedSlowTask.h"
#include "Core/Public/Misc/MessageDialog.h"
#include "Engine/World.h"
#include "SCleanProjectAssetDialog.h"
#include "CleanProjectSettings.h"
#include "CleanProjectGameSettings.h"
#include "Misc/FileHelper.h"
#include "AssetToolsModule.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace CleanProjectOperations
{
	void CheckDependenciesBasedOn(TArray<FAssetData> SelectedAssets)
	{
		CheckDependenciesInternal(GetAllGameAssets(), SelectedAssets);
	}

	void CheckDependenciesOf(TArray<FAssetData> SelectedAssets)
	{
		// Remove the selected from all the game assets before proceding.
		TArray<FAssetData> FilteredGameAssets = GetAllGameAssets();
		FilteredGameAssets.RemoveAllSwap([&SelectedAssets](const FAssetData& current)
			{
				return SelectedAssets.Contains(current);
			});

		CheckDependenciesInternal(SelectedAssets, FilteredGameAssets);
	}

	void CheckDependenciesInternal(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest)
	{
		auto Settings = GetDefault<UCleanProjectGameSettings>();

		for (const FName& WhitelistAssetPath : Settings->WhitelistAssetsPaths)
		{
			AssetsToTest.RemoveAllSwap([&WhitelistAssetPath](const FAssetData& AssetData)
				{
					return AssetData.ObjectPath == WhitelistAssetPath;
				});
		}

		for (auto PackageIt = DependenciesToTest.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			AssetsToTest.RemoveSwap(*PackageIt);
		}

		// Recursively get all the packages to check from dependencies.
		TSet<FName> AllPackageNamesToCheck;

		// Gather the dependencies of the whitelisted assets as well if needed.
		if (Settings->bCheckWhitelistReferences)
		{
			for (const FName& WhitelistAssetPath : Settings->WhitelistAssetsPaths)
			{
				AllPackageNamesToCheck.Add(WhitelistAssetPath);
			}
		}
		
		// Create a slow task to display a progressbar for the user.
		FScopedSlowTask SlowTask(DependenciesToTest.Num(), LOCTEXT("CleanProject_SlowTaskTitle", "Gathering Dependencies..."));
		bool showCancelButton = true;
		bool allowPIE = true;
		SlowTask.MakeDialog(showCancelButton, allowPIE);

		for (auto PackageIt = DependenciesToTest.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			SlowTask.EnterProgressFrame();

			if (!AllPackageNamesToCheck.Contains(PackageIt->PackageName))
			{
				AllPackageNamesToCheck.Add(PackageIt->PackageName);
				RecursiveGetDependencies(PackageIt->PackageName, AllPackageNamesToCheck);
			}
		}
		
		// Removed the dependenices found from the ones tested.
		for (auto DependsIt = AllPackageNamesToCheck.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			const FName& DependencyPackageName = *DependsIt;

			AssetsToTest.RemoveAllSwap([&DependencyPackageName](const FAssetData& AssetData)
				{
					return AssetData.PackageName == DependencyPackageName;
				});
		}

		// Confirm that there is at least one package to 
		if (AssetsToTest.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CleanProject_NoFilesToDelete", "No unused assets found."));
		}
		else if (AllPackageNamesToCheck.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CleanProject_NoFilesChecked", "No dependency files were selected."));
		}
		else
		{
			SCleanProjectAssetDialog::OpenAssetDialog(AssetsToTest);
		}
	}

	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FName> Dependencies;

		AssetRegistryModule.Get().GetDependencies(PackageName, Dependencies);

		for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			if (!AllDependencies.Contains(*DependsIt))
			{
				//TODO: Make the skippable folders configurable.
				const bool bIsEnginePackage = (*DependsIt).ToString().StartsWith(TEXT("/Engine"));
				const bool bIsScriptPackage = (*DependsIt).ToString().StartsWith(TEXT("/Script"));
				if (!bIsEnginePackage && !bIsScriptPackage)
				{
					AllDependencies.Add(*DependsIt);
					RecursiveGetDependencies(*DependsIt, AllDependencies);
				}
			}
		}
	}

	void GenerateBlacklist(const TArray<FAssetData>& AssetsToBlacklist, const bool bAppend, const FString& Platform /*= ""*/, const FString& Configuration /*= ""*/)
	{
		auto Settings = GetDefault<UCleanProjectSettings>();
		
		const EFileWrite WriteFlags = bAppend ? EFileWrite::FILEWRITE_Append : EFileWrite::FILEWRITE_None;
		TArray<FString> SelectedConfigurations = GetListFromSelection(Settings->BlacklistFiles, Configuration);
		TArray<FString> SelectedPlatforms = GetListFromSelection(Settings->PlatformsPaths, Platform);
		
		FString FileContent;
		for (const FAssetData& AssetData : AssetsToBlacklist)
		{
			FString assetPath = AssetData.PackageName.ToString();
			FileContent += FString::Printf(TEXT("../../..%s\n"), *assetPath);
		}
		
		if (Settings->bUseSmartBlackList)
		{
			FString projectBuildRoot = FPaths::ProjectDir() + "Build";
		
			for (const FString& platformFolder : SelectedPlatforms)
			{
				for (const FString& listFile : SelectedConfigurations)
				{
					FString slash = FGenericPlatformMisc::GetDefaultPathSeparator();
					FString platformPath = projectBuildRoot + slash + platformFolder + slash + listFile;
					FFileHelper::SaveStringToFile(FileContent, *platformPath, FFileHelper::EEncodingOptions::AutoDetect, 
						&IFileManager::Get(), WriteFlags);
				}
			}
		}
		else
		{
			FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Blacklist.txt");
		
			FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect,
				&IFileManager::Get(), WriteFlags);
			FPlatformProcess::LaunchURL(*FString::Printf(TEXT("file://%s"), *FilePath), NULL, NULL);
		}
	}

    void FixUpRedirectorsInProject()
    {
		TArray<UObject*> AssetsToRedirect;
		bool bLoadSuccess = LoadRedirectAssetsInProject(AssetsToRedirect);

        if (bLoadSuccess)
        {
            // Transform Objects array to ObjectRedirectors array
            TArray<UObjectRedirector*> Redirectors;
            for (auto Object : AssetsToRedirect)
            {
                auto Redirector = CastChecked<UObjectRedirector>(Object);
                Redirectors.Add(Redirector);
            }

            // Load the asset tools module
            FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
            AssetToolsModule.Get().FixupReferencers(Redirectors);
        }
    }

    void DeleteEmptyProjectFolders()
    {
		TArray<FString> EmptyFoldersFound;
		GetEmptyFolderInPath(FPaths::ProjectContentDir(), EmptyFoldersFound);

		for (const FString& Folder : EmptyFoldersFound)
		{
			IFileManager::Get().DeleteDirectory(*Folder, false, true);
		}
    }

    TArray<FAssetData> GetAllGameAssets(TArray<FName> ClassTypes /* = TArray<FName>()*/)
	{
		TArray<FAssetData> AllAssetData;

		FARFilter Filter;
		Filter.PackagePaths.Add(TEXT("/Game"));
		Filter.bRecursivePaths = true;

		Filter.ClassNames.Reserve(ClassTypes.Num());
		for (const auto& classType : ClassTypes)
		{
			Filter.ClassNames.Add(classType);
		}
		
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

		return AllAssetData;
	}

	TArray<FAssetData> GetAllMapAssets()
	{
		TArray<FName> MapsClassFilter;
		MapsClassFilter.Add(UWorld::StaticClass()->GetFName());

		return GetAllGameAssets(MapsClassFilter);
	}

	TArray<FString> GetListFromSelection(const TArray<FString>& List, const FString& Selection)
	{
		TArray<FString> Result;
		if (List.Contains(Selection))
		{
			Result.Add(Selection);
		}
		else
		{
			Result = List;
		}

		return Result;
	}

    bool LoadRedirectAssetsInProject(TArray<UObject*>& Objects)
    {
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

        // Form a filter from the paths
        FARFilter Filter;
        Filter.bRecursivePaths = true;
        {
            Filter.PackagePaths.Emplace(TEXT("/Game"));
            Filter.ClassNames.Emplace(TEXT("ObjectRedirector"));
        }

        // Query for a list of assets in the selected paths
        TArray<FAssetData> AssetList;
        AssetRegistryModule.Get().GetAssets(Filter, AssetList);

        bool bAllAreLoaded = true;
        
		for (const auto& Asset : AssetList)
		{
			UObject* FoundObject = FindObject<UObject>(NULL, *Asset.ObjectPath.ToString());
			if (FoundObject)
			{
				Objects.Add(FoundObject);
			}
			else
			{
				UObject* LoadedObject = LoadObject<UObject>(NULL, *Asset.ObjectPath.ToString(), NULL, LOAD_None, NULL);
				if (LoadedObject)
				{
					Objects.Add(LoadedObject);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to fixup redirects."));
					bAllAreLoaded = false;
				}
			}
		}

		return bAllAreLoaded;
    }

    void GetEmptyFolderInPath(const FString& BaseDirectory, TArray<FString>& OutEmptyFolders)
    {
        struct FEmptyFolderVisitor : public IPlatformFile::FDirectoryVisitor
        {
			TArray<FString>& EmptyFolders;
			const FString& CurrentDirectory;
            bool bIsEmpty;

            FEmptyFolderVisitor(TArray<FString>& InEmptyFolders, const FString& InCurrentDirectory)
                : EmptyFolders(InEmptyFolders)
				, CurrentDirectory(InCurrentDirectory)
				, bIsEmpty(true)
            {
            }

            virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
            {
				if (bIsDirectory)
				{
					FString DirectoryName(FilenameOrDirectory);
					EmptyFolders.Add(DirectoryName);

					GetEmptyFolderInPath(DirectoryName, EmptyFolders);
				}
				else
				{
					EmptyFolders.Remove(CurrentDirectory);
                    bIsEmpty = false;
				}

                return true;
            }
        };

		FEmptyFolderVisitor EmptyFolderVisitor(OutEmptyFolders, BaseDirectory);
        IFileManager::Get().IterateDirectoryRecursively(*BaseDirectory, EmptyFolderVisitor);
    }
}

#undef LOCTEXT_NAMESPACE