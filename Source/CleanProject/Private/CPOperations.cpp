// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.
#include "CPOperations.h"

#include "SCPAssetDialog.h"

#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "CleanProject"

namespace OperationsHelpers
{
	void RemoveAllAssetsByName(TArray<FAssetData>& AssetsList, const TArray<FName>& AssetsNameToRemove)
	{
		for (const FName& WhitelistAssetPath : AssetsNameToRemove)
		{
			AssetsList.RemoveAllSwap([&WhitelistAssetPath](const FAssetData& AssetData)
				{
					return AssetData.ObjectPath == WhitelistAssetPath;
				});
		}
	}

	void RemoveAllAssetsByName(TArray<FAssetData>& AssetsList, const TSet<FName>& AssetsNameToRemove)
	{
		RemoveAllAssetsByName(AssetsList, AssetsNameToRemove.Array());
	}

	void RemoveAllAssets(TArray<FAssetData>& AssetsList, const TArray<FAssetData>& AssetsToRemove)
	{
		TArray<FName> AssetsPaths;
		for (auto PackageIt = AssetsToRemove.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			AssetsPaths.Add(PackageIt->ObjectPath);
		}

		RemoveAllAssetsByName(AssetsList, AssetsPaths);
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

	bool LoadRedirectAssetsInProject(const TArray<FAssetData>& RedirectorsList, TArray<UObjectRedirector*>& Objects)
	{
		bool bAllAreLoaded = true;
		for (const auto& RedirectorAsset : RedirectorsList)
		{
			const FString ObjectPath = RedirectorAsset.ObjectPath.ToString();
			UObjectRedirector* FoundObject = FindObject<UObjectRedirector>(NULL, *ObjectPath);
			if (FoundObject)
			{
				Objects.Add(FoundObject);
			}
			else
			{
				UObjectRedirector* LoadedObject = LoadObject<UObjectRedirector>(NULL, *ObjectPath, NULL, LOAD_None, NULL);
				if (LoadedObject)
				{
					Objects.Add(LoadedObject);
				}
				else
				{
					UE_LOG(LogCleanProject, Warning, TEXT("Failed to fixup redirects. Redirect %s - did not load."), *ObjectPath);
					bAllAreLoaded = false;
				}
			}
		}

		return bAllAreLoaded;
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
}

namespace CPOperations
{
	void CheckDependenciesOf(TArray<FAssetData> SelectedAssets)
	{
		CheckDependenciesInternal(SelectedAssets, GetAllGameAssets<UWorld>());
	}

	void CheckDependenciesInternal(TArray<FAssetData> AssetsToTest, TArray<FAssetData> DependenciesToTest)
	{
		auto Settings = GetDefault<UCPProjectSettings>();
		
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, Settings->WhitelistAssetsPaths);
		OperationsHelpers::RemoveAllAssets(AssetsToTest, DependenciesToTest);
		
		// Collect all the names we want to check dependencies for.
		TSet<FName> PackageNameToCheck;
		{
			if (Settings->bCheckWhitelistReferences)
			{
				for (const FName& WhitelistAssetPath : Settings->WhitelistAssetsPaths)
				{
					PackageNameToCheck.Add(WhitelistAssetPath);
				}
			}
			for (const FAssetData& AssetData : DependenciesToTest)
			{
				PackageNameToCheck.Add(AssetData.PackageName);
			}
		}

		// Check the dependencies of the collected packages with a progressbar.
		TSet<FName> PackageNamesChecked;
		{
			FScopedSlowTask SlowTask(DependenciesToTest.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
			bool bShowCancelButton = true;
			bool bAllowPIE = true;
			SlowTask.MakeDialog(bShowCancelButton, bAllowPIE);

			for (const FName& CurrentPackageName : PackageNameToCheck)
			{
				const FText CurrentAssetName = FText::FromName(CurrentPackageName);
				const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
				SlowTask.EnterProgressFrame(1.f, CurrentAssetText);

				// Check if we already found the dependencies for this asset, if not, get them now.
				if (!PackageNamesChecked.Contains(CurrentPackageName))
				{
					PackageNamesChecked.Add(CurrentPackageName);
					RecursiveGetDependencies(CurrentPackageName, PackageNamesChecked);
				}
			}
		}

		// Removed the dependenices found from the ones tested.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, PackageNamesChecked);
		
		// Open dialogs based on the results.
		{
			if (PackageNamesChecked.Num() == 0)
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilesChecked", "No files were checked."));
			}
			else if (AssetsToTest.Num() == 0)
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilesToDelete", "No unused assets found."));
			}
			else
			{
				SCPAssetDialog::OpenAssetDialog(AssetsToTest);
			}
		}	
	}

	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FName> Dependencies;

		AssetRegistryModule.GetDependencies(PackageName, Dependencies);

		for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			//TODO: Make the skippable folders configurable.
			const bool bIsEnginePackage = (*DependsIt).ToString().StartsWith(TEXT("/Engine"));
			const bool bIsScriptPackage = (*DependsIt).ToString().StartsWith(TEXT("/Script"));

			if (!AllDependencies.Contains(*DependsIt) && !bIsEnginePackage && !bIsScriptPackage)
			{
				AllDependencies.Add(*DependsIt);
				RecursiveGetDependencies(*DependsIt, AllDependencies);
			}
		}
	}

	void GenerateBlacklist(const TArray<FAssetData>& AssetsToBlacklist, const bool bAppend, const FString& Platform /*= ""*/, const FString& Configuration /*= ""*/)
	{
		auto Settings = GetDefault<UCPEditorSettings>();
		
		const EFileWrite WriteFlags = bAppend ? EFileWrite::FILEWRITE_Append : EFileWrite::FILEWRITE_None;
		const TArray<FString> SelectedConfigurations = OperationsHelpers::GetListFromSelection(Settings->BlacklistFiles, Configuration);
		const TArray<FString> SelectedPlatforms = OperationsHelpers::GetListFromSelection(Settings->PlatformsPaths, Platform);
		
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
		TArray<FAssetData> RedirectorAssetList = GetAllGameAssets<UObjectRedirector>();
		TArray<UObjectRedirector*> AssetsToRedirect;
		bool bLoadSuccess = OperationsHelpers::LoadRedirectAssetsInProject(RedirectorAssetList, AssetsToRedirect);

        if (bLoadSuccess)
        {
            FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
            AssetToolsModule.Get().FixupReferencers(AssetsToRedirect);
        }
		else
		{
			UE_LOG(LogCleanProject, Warning, TEXT("Fixup redirects will not be attempted, at least one failed."));
		}
    }

    void DeleteEmptyProjectFolders()
    {
		TArray<FString> EmptyFoldersFound;
		OperationsHelpers::GetEmptyFolderInPath(FPaths::ProjectContentDir(), EmptyFoldersFound);

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
}

#undef LOCTEXT_NAMESPACE