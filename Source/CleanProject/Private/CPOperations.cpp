// Copyright Out-of-the-Box Plugins 2018-2021. All Rights Reserved.

#include "CPOperations.h"
#include "Widgets/SCPAssetDialog.h"
#include "CPLog.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "CPSettings.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/MessageDialog.h"
#include "Engine/Level.h"

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
					const FString DirectoryName(FilenameOrDirectory);
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
		if (EmptyFolderVisitor.bIsEmpty)
		{
			OutEmptyFolders.Add(BaseDirectory);
		}
	}

	bool LoadRedirectAssetsInProject(const TArray<FAssetData>& RedirectsList, TArray<UObjectRedirector*>& Objects)
	{
		bool bAllAreLoaded = true;
		for (const auto& RedirectAsset : RedirectsList)
		{
			const FString ObjectPath = RedirectAsset.ObjectPath.ToString();
			UObjectRedirector* FoundObject = FindObject<UObjectRedirector>(nullptr, *ObjectPath);
			if (FoundObject)
			{
				Objects.Add(FoundObject);
			}
			else
			{
				UObjectRedirector* LoadedObject = LoadObject<UObjectRedirector>(nullptr, *ObjectPath, nullptr, LOAD_None, nullptr);
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

	TArray<FAssetData> CheckForUnusedAssets()
	{
		const TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
		return CheckForUnusedAssets(AllAssets);
	}

	TArray<FAssetData> CheckForUnusedAssets(TArray<FAssetData> AssetsToTest)
	{
		const UCPSettings* Settings = GetDefault<UCPSettings>();
		// Collect all the names we want to check dependencies for.
		TSet<FName> PackageNameToCheck;
		{
			if (Settings->bCheckWhitelistReferences)
			{
				for (const FName& WhitelistAssetPath : Settings->GetWhitelistAssetsPaths())
				{
					PackageNameToCheck.Add(WhitelistAssetPath);
				}
			}
			if (Settings->bCheckAllMapsReferences)
			{
				for (const FAssetData& WorldAsset : GetAllGameAssets<UWorld>())
				{
					PackageNameToCheck.Add(WorldAsset.PackageName);
				}
			}
		}

		// Remove the whitelisted assets regardless if they are used selected for dependencies or not.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, Settings->GetWhitelistAssetsPaths());

		// Remove the packages we are going to check dependencies for from the assets to test, because they depend on themselves.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, PackageNameToCheck);

		// Check the dependencies of the collected packages with a progressbar.
		const FTreeAssetDependency AssetsDependencies = GetAssetDependenciesTree(PackageNameToCheck.Array());

		// Removed the dependencies found from the ones tested.
		OperationsHelpers::RemoveAllAssetsByName(AssetsToTest, AssetsDependencies);

		return AssetsToTest;
	}

	void CheckAllDependencies()
	{
		const TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
		CheckDependenciesOf(AllAssets);
	}

	void CheckDependenciesOf(const TArray<FAssetData> SelectedAssets)
	{
		const TArray<FAssetData> UnusedAssets = CheckForUnusedAssets(SelectedAssets);
		// Open dialogs based on the results.
		if (UnusedAssets.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilesToDelete", "No unused assets found."));
		}
		else
		{
			SCPAssetDialog::OpenAssetDialog(UnusedAssets);
		}
	}

	int64 GetAssetDiskSize(const FAssetData& Asset)
	{
		const FName PackageName = FName(*Asset.GetPackage()->GetName());
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
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
			//FScopedSlowTask SlowTask(AssetsList.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
			const bool bShowCancelButton = true;
			const bool bAllowPie = false;
			//SlowTask.MakeDialog(bShowCancelButton, bAllowPie);

			for (const FAssetData& AssetDataReported : AssetsList)
			{
				const FText CurrentAssetName = FText::FromName(AssetDataReported.PackageName);
				const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
				//SlowTask.EnterProgressFrame(1.f, CurrentAssetText);
				TotalDiskSize += GetAssetDiskSize(AssetDataReported);
			}
		}

		return TotalDiskSize;
	}

	int64 GetUnusedAssetsDiskSize(TArray<FAssetData> AssetsToTest)
	{
		const TArray<FAssetData> UnusedAssets = CheckForUnusedAssets(AssetsToTest);
		return GetAssetsDiskSize(UnusedAssets);
	}

	int64 GetUnusedAssetsDiskSize()
	{
		const TArray<FAssetData> AllAssets = CPOperations::GetAllGameAssets();
		return GetUnusedAssetsDiskSize(AllAssets);
	}

	FTreeAssetDependency GetAssetDependenciesTree(const TArray<FAssetDataPtr>& AssetsNameList)
	{
		TArray<FName> AssetsNames;
		for (const auto& AssetNamePtr : AssetsNameList)
		{
			AssetsNames.Add(*AssetNamePtr);
		}

		return GetAssetDependenciesTree(AssetsNames);
	}

	FTreeAssetDependency GetAssetDependenciesTree(const TArray<FName>& AssetsNameList)
	{
		FTreeAssetDependency Result;
		for (const auto& AssetName : AssetsNameList)
		{
			Result.AddTopLevelDependency(AssetName);
		}

		//FScopedSlowTask SlowTask(AssetsNameList.Num(), LOCTEXT("SlowTaskTitle", "Gathering Dependencies..."));
		const bool bShowCancelButton = true;
		const bool bAllowPie = false;
		//SlowTask.MakeDialog(bShowCancelButton, bAllowPie);
		TSet<FName> PackageNamesChecked;
		for (const FChildDependency& TopDependency : Result.TopLevelDependencies)
		{
			const FName DependencyName = TopDependency.GetAssetName();
			const FText CurrentAssetName = FText::FromName(DependencyName);
			const FText CurrentAssetText = FText::Format(LOCTEXT("CurrentAsset", "Current Asset: {0}"), CurrentAssetName);
			//SlowTask.EnterProgressFrame(1.f, CurrentAssetText);
			if (!PackageNamesChecked.Contains(DependencyName))
			{
				PackageNamesChecked.Add(DependencyName);
				RecursiveGetDependencies(DependencyName, PackageNamesChecked, Result);
			}
		}

		return Result;
	}

	void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, FTreeAssetDependency& ResultTreeDependency)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FName> Dependencies;
		AssetRegistryModule.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::All);
		TArray<FAssetData> Assets;
		if (AssetRegistryModule.Get().GetAssetsByPackageName(PackageName, Assets))
		{
			for (const FAssetData& AssetData : Assets)
			{
				if (AssetData.GetClass() && AssetData.GetClass()->IsChildOf<UWorld>())
				{
					TArray<FString> ExternalActorsPackages = ULevel::GetOnDiskExternalActorPackages(PackageName.ToString());
					for (const FString& ExternalActorsPackage : ExternalActorsPackages)
					{
						FName ExternalActorPackageName = FName(ExternalActorsPackage);
						ResultTreeDependency.AddDependency(PackageName, ExternalActorPackageName);
						AllDependencies.Add(ExternalActorPackageName);
						RecursiveGetDependencies(ExternalActorPackageName, AllDependencies, ResultTreeDependency);
					}
				}
			}
		}

		for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			const FName DependencyName = *DependsIt;
			const bool bIsEnginePackage = (DependencyName).ToString().StartsWith(TEXT("/Engine"));
			const bool bIsScriptPackage = (DependencyName).ToString().StartsWith(TEXT("/Script"));

			if (!AllDependencies.Contains(DependencyName) && !bIsEnginePackage && !bIsScriptPackage)
			{
				ResultTreeDependency.AddDependency(PackageName, DependencyName);
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

		if (Settings->bSaveToTempFile)
		{
			const FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Blacklist.txt");

			FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect,
				&IFileManager::Get(), WriteFlags);
			FPlatformProcess::LaunchURL(*FString::Printf(TEXT("file://%s"), *FilePath), nullptr, nullptr);
		}
		else
		{
			const FString ProjectBuildRoot = FPaths::ProjectDir() + "Build";
			for (const FString& platformFolder : SelectedPlatforms)
			{
				for (const FString& listFile : SelectedConfigurations)
				{
					FString slash = FGenericPlatformMisc::GetDefaultPathSeparator();
					FString platformPath = ProjectBuildRoot + slash + platformFolder + slash + listFile;
					FFileHelper::SaveStringToFile(FileContent, *platformPath, FFileHelper::EEncodingOptions::AutoDetect,
						&IFileManager::Get(), WriteFlags);
				}
			}
		}
	}

	void FixUpRedirectsInProject()
	{
		const TArray<FAssetData> RedirectAssetList = GetAllGameAssets<UObjectRedirector>();
		TArray<UObjectRedirector*> AssetsToRedirect;
		const bool bLoadSuccess = OperationsHelpers::LoadRedirectAssetsInProject(RedirectAssetList, AssetsToRedirect);
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
		const FString& ContentDirectory = FPaths::ProjectContentDir();
		DeleteFolderByPath(ContentDirectory);
	}

	void DeleteEmptyProjectFolders(TArray<FString> SelectedFolders)
	{
		for (const FString& SelectedFolder : SelectedFolders)
		{
			FString ConvertedName;
			if (FPackageName::TryConvertLongPackageNameToFilename(SelectedFolder, ConvertedName))
			{
				const FString FolderName = ConvertedName + TEXT("/");
				DeleteFolderByPath(FolderName);
			}
		}
	}

	void DeleteFolderByPath(const FString& FolderPath)
	{
		FixUpRedirectsInProject();
		TArray<FString> EmptyFoldersFound;
		OperationsHelpers::GetEmptyFolderInPath(FolderPath, EmptyFoldersFound);
		for (const FString& FolderToDelete : EmptyFoldersFound)
		{
			IFileManager::Get().DeleteDirectory(*FolderToDelete, false, true);
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

	bool FChildDependency::operator!=(const FChildDependency& Other) const
	{
		return !GetAssetName().IsEqual(Other.GetAssetName());
	}

	FChildDependency INVALID_CHILD(FName("INVALID_CHILD"));

	FChildDependency::FChildDependency(const FName& InAssetName)
	{
		AssetName = MakeShared<FName>(InAssetName);
	}

	void FChildDependency::AddDependency(const FName& ChildDependency)
	{
		ChildDependencies.Emplace(ChildDependency);
	}

	TArray<FAssetDataPtr> FChildDependency::GetChildrenAssetPtrs()
	{
		TArray<FAssetDataPtr> Children;
		for (FChildDependency& Dependency : ChildDependencies)
		{
			Children.Emplace(Dependency.AssetName);
		}

		return Children;
	}

	void FTreeAssetDependency::AddTopLevelDependency(const FName& AssetName)
	{
		FChildDependency& NewChild = TopLevelDependencies.Emplace_GetRef(AssetName);
		TopLevelAssetsPtr.Emplace(NewChild.AssetName);
	}

	FChildDependency& FTreeAssetDependency::GetDependencyRecursive(const FName& OwnerName, TArray<FChildDependency>& ChildrenToCheck)
	{
		for (FChildDependency& ChildToCheck : ChildrenToCheck)
		{
			if (ChildToCheck.GetAssetName() == OwnerName)
			{
				return ChildToCheck;
			}
			else
			{
				FChildDependency& ChildFound = GetDependencyRecursive(OwnerName, ChildToCheck.ChildDependencies);

				if (ChildFound != INVALID_CHILD)
				{
					return ChildFound;
				}
			}
		}

		return INVALID_CHILD;
	}

	void FTreeAssetDependency::GatherDependencyRecursive(TArray<FName>& OutResult, const TArray<FChildDependency>& ChildrenToCheck) const
	{
		for (const FChildDependency& ChildToCheck : ChildrenToCheck)
		{
			OutResult.Add(ChildToCheck.GetAssetName());
			GatherDependencyRecursive(OutResult, ChildToCheck.ChildDependencies);
		}
	}

	void FTreeAssetDependency::AddDependency(const FName& OwnerName, const FName& DependencyName)
	{
		FChildDependency& OwnerDependency = (*this)[OwnerName];
		OwnerDependency.AddDependency(DependencyName);
	}

	TArray<FName> FTreeAssetDependency::GetDependenciesAsList() const
	{
		TArray<FName> AllDependencies;
		GatherDependencyRecursive(AllDependencies, TopLevelDependencies);
		return AllDependencies;
	}
}

#undef LOCTEXT_NAMESPACE
