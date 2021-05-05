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
					return AssetData.PackageName == WhitelistAssetPath;
				});
		}
	}

	void RemoveAllAssetsByName(TArray<FAssetData>& AssetsList, const TSet<FName>& AssetsNameToRemove)
	{
		RemoveAllAssetsByName(AssetsList, AssetsNameToRemove.Array());
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
	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths)
	{
		FARFilter Filter;
		Filter.bRecursivePaths = true;

		for (const FString& FolderPath : FolderPaths)
		{
			Filter.PackagePaths.Add(FName(FolderPath));
		}

		TArray<FAssetData> AllAssetData;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetAssets(Filter, AllAssetData);

		return AllAssetData;
	}

	TArray<FAssetData> CheckForUnusuedAssets()
	{
		TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
		return CheckForUnusuedAssets(AllAssets);
	}

	TArray<FAssetData> CheckForUnusuedAssets(TArray<FAssetData> AssetsToTest)
	{
		const UCPSettings* Settings = GetDefault<UCPSettings>();

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
			if (Settings->bCheckAllMapsRefernece)
			{
				for (const FAssetData& WorldAsset : GetAllGameAssets<UWorld>())
				{
					PackageNameToCheck.Add(WorldAsset.PackageName);
				}
			}
		}

		// Remove the whitelisted assets regardless if they are used selected for depedencies or not.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, Settings->WhitelistAssetsPaths);

		// Remove the packages we are going to check dependencies for from the assets to test, because they depend on themselves.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, PackageNameToCheck);

		// Check the dependencies of the collected packages with a progressbar.
		FTreeAssetDepedency AssetsDependencies = GetAssetDependenciesTree(PackageNameToCheck.Array());

		// Removed the dependenices found from the ones tested.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, AssetsDependencies);

		return AssetsToTest;
	}

	void CheckAllDependencies()
	{
		TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
		CheckDependenciesOf(AllAssets);
	}

	void CheckDependenciesOf(TArray<FAssetData> AssetsToTest)
	{
		TArray<FAssetData> UnusuedAssets = CheckForUnusuedAssets(AssetsToTest);
		
		// Open dialogs based on the results.
		if (UnusuedAssets.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilesToDelete", "No unused assets found."));
		}
		else
		{
			SCPAssetDialog::OpenAssetDialog(UnusuedAssets);
		}
	}

	int64 GetAssetDiskSize(const FAssetData& Asset)
	{
		const FName PackageName = FName(*Asset.GetPackage()->GetName());

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		if (const FAssetPackageData* PackageData = AssetRegistryModule.Get().GetAssetPackageData(PackageName))
		{
			return PackageData->DiskSize;
		}
		return -1;
	}

	int64 GetAssetsDiskSize(const TArray<FAssetData>& AssetsList)
	{
		int64 TotalDiskSize = 0;
		{
			FScopedSlowTask SlowTask(AssetsList.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
			bool bShowCancelButton = true;
			bool bAllowPIE = false;
			SlowTask.MakeDialog(bShowCancelButton, bAllowPIE);

			for (const FAssetData& AssetDataReported : AssetsList)
			{
				const FText CurrentAssetName = FText::FromName(AssetDataReported.PackageName);
				const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
				SlowTask.EnterProgressFrame(1.f, CurrentAssetText);

				TotalDiskSize += GetAssetDiskSize(AssetDataReported);
			}
		}
		
		return TotalDiskSize;
	}

	int64 GetUnusuedAssetsDiskSize(TArray<FAssetData> AssetsToTest)
	{
		TArray<FAssetData> UnusuedAssets = CheckForUnusuedAssets(AssetsToTest);
		return GetAssetsDiskSize(UnusuedAssets);
	}

	int64 GetUnusuedAssetsDiskSize()
	{
		TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
		return GetUnusuedAssetsDiskSize(AllAssets);
	}

	FTreeAssetDepedency GetAssetDependenciesTree(const TArray<FAssetDataPtr>& AssetsNameList)
	{
		TArray<FName> AssetsNames;
		for (const auto& AssetNamePtr : AssetsNameList)
		{
			AssetsNames.Add(*AssetNamePtr);
		}

		return GetAssetDependenciesTree(AssetsNames);
	}

	FTreeAssetDepedency GetAssetDependenciesTree(const TArray<FName>& AssetsNameList)
	{
		FTreeAssetDepedency Result;

		for (const auto& AssetName : AssetsNameList)
		{
			Result.AddTopLevelDependency(AssetName);
		}

		FScopedSlowTask SlowTask(AssetsNameList.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
		bool bShowCancelButton = true;
		bool bAllowPIE = false;
		SlowTask.MakeDialog(bShowCancelButton, bAllowPIE);

		TSet<FName> PackageNamesChecked;
		for (const FChildDepedency& TopDependency : Result.TopLevelDependencies)
		{
			const FName DependencyName = TopDependency.GetAssetName();

			const FText CurrentAssetName = FText::FromName(DependencyName);
			const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
			SlowTask.EnterProgressFrame(1.f, CurrentAssetText);

			if (!PackageNamesChecked.Contains(DependencyName))
			{
				PackageNamesChecked.Add(DependencyName);
				RecursiveGetDependencies(DependencyName, PackageNamesChecked, Result);
			}
		}

		return Result;
	}

	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, FTreeAssetDepedency& ResultTreeDependency)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		TArray<FName> Dependencies;
		AssetRegistryModule.GetDependencies(PackageName, Dependencies);

		for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			const FName DependencyName = *DependsIt;

			//TODO: Make the skippable folders configurable.
			const bool bIsEnginePackage = (DependencyName).ToString().StartsWith(TEXT("/Engine"));
			const bool bIsScriptPackage = (DependencyName).ToString().StartsWith(TEXT("/Script"));

			if (!AllDependencies.Contains(DependencyName) && !bIsEnginePackage && !bIsScriptPackage)
			{
				ResultTreeDependency.AddDepedency(PackageName, DependencyName);
				AllDependencies.Add(DependencyName);
				RecursiveGetDependencies(DependencyName, AllDependencies, ResultTreeDependency);
			}
		}
	}

	void GenerateBlacklist(const TArray<FAssetData>& AssetsToBlacklist, const bool bAppend, const FString& Platform /*= ""*/, const FString& Configuration /*= ""*/)
	{
		const UCPSettings* Settings = GetMutableDefault<UCPSettings>();
		
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
		FixUpRedirectorsInProject();

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

	bool FChildDepedency::operator!=(const FChildDepedency& Other)
	{
		return !GetAssetName().IsEqual(Other.GetAssetName());
	}

	FChildDepedency INVALID_CHILD(FName("INVALID_CHILD"));

	FChildDepedency::FChildDepedency(const FName& InAssetName) 
	{
		AssetName = MakeShared<FName>(InAssetName);
	}

	void FChildDepedency::AddDependency(const FName& ChildDependency)
	{
		ChildDependencies.Emplace(ChildDependency);
	}

	TArray<FAssetDataPtr> FChildDepedency::GetChildrenAssetPtrs()
	{
		TArray<FAssetDataPtr> Children;
		for (FChildDepedency& Dependency : ChildDependencies)
		{
			Children.Emplace(Dependency.AssetName);
		}

		return Children;
	}

	void FTreeAssetDepedency::AddTopLevelDependency(const FName& AssetName)
	{
		FChildDepedency& NewChild = TopLevelDependencies.Emplace_GetRef(AssetName);
		TopLevelAssetsPtr.Emplace(NewChild.AssetName);
	}

	FChildDepedency& FTreeAssetDepedency::GetDependencyRecursive(const FName& OwnerName, TArray<FChildDepedency>& ChildrenToCheck)
	{
		for (FChildDepedency& ChildToCheck : ChildrenToCheck)
		{
			if (ChildToCheck.GetAssetName() == OwnerName)
			{
				return ChildToCheck;
			}
			else
			{
				FChildDepedency& ChildFound = GetDependencyRecursive(OwnerName, ChildToCheck.ChildDependencies);

				if (ChildFound != INVALID_CHILD)
				{
					return ChildFound;
				}
			}
		}

		return INVALID_CHILD;
	}

	void FTreeAssetDepedency::GatherDependencyRecursive(TArray<FName>& OutResult, const TArray<FChildDepedency>& ChildrenToCheck) const
	{
		for (const FChildDepedency& ChildToCheck : ChildrenToCheck)
		{
			OutResult.Add(ChildToCheck.GetAssetName());
			GatherDependencyRecursive(OutResult, ChildToCheck.ChildDependencies);
		}
	}

	void FTreeAssetDepedency::AddDepedency(const FName& OwnerName, const FName& DependencyName)
	{
		FChildDepedency& OwnerDependency = (*this)[OwnerName];
		OwnerDependency.AddDependency(DependencyName);
	}

	TArray<FName> FTreeAssetDepedency::GetDependenciesAsList() const
	{
		TArray<FName> AllDependencies;
		GatherDependencyRecursive(AllDependencies, TopLevelDependencies);
		return AllDependencies;
	}
}

#undef LOCTEXT_NAMESPACE