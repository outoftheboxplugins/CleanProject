// Copyright Out-of-the-Box Plugins 2018-2023. All Rights Reserved.

#pragma once

namespace CPHelpers
{
	/**
	 * @brief Gets all the assets from the Game's content folder. Optionally filters them by a class type
	 * @param ClassFilter Class we want to filter assets for. If unset no filtering will be done and assets will be returned
	 * @return List of all the assets found
	 */
	TSet<FAssetData> GetAllGameAssets(TOptional<FTopLevelAssetPath> ClassFilter = {});
	/**
	 * @brief Gets all the assets of the input type from the Game's content folder
	 * @tparam T Type of asset we want to to look for
	 * @return List of all the assets found
	 */
	template <typename T>
	TSet<FAssetData> GetAllGameAssetsOfType();
	/**
	 * @brief Recursively gets all the assets from the input folders
	 * @param FolderPaths Folders we want to recursively get all assets from
	 * @return List of all the assets found inside the input Folders
	 */
	TArray<FAssetData> GetAssetsInPaths(TArray<FString> FolderPaths);
	/**
	 * @brief Recursively gets all the assets from the input folder
	 * @param FolderPath Folder we want to recursively get all assets from
	 * @return List of all the aseets we found inside the input Folder
	 */
	TArray<FAssetData> GetAssetsInPaths(FString FolderPath);
	/**
	 * @brief Scans the Config/ folder .ini files for any potentially referenced assets
	 * @return List of all the asset datas found inside the .ini files
	 */
	TArray<FAssetData> GetAssetsInIniFiles();
} // namespace CPHelpers

template <typename T>
TSet<FAssetData> CPHelpers::GetAllGameAssetsOfType()
{
	return GetAllGameAssets(T::StaticClass()->GetClassPathName());
}
