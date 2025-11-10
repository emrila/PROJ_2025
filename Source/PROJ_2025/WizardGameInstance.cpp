// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardGameInstance.h"

#include "AssetRegistry/AssetRegistryModule.h"

TArray<URoomData*> UWizardGameInstance::GetAllRoomData() const
{
	TArray<URoomData*> Result;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(URoomData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName("/Game/RoomData")); 
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	for (const FAssetData& Asset : AssetList)
	{
		if (URoomData* RoomData = Cast<URoomData>(Asset.GetAsset()))
		{
			Result.Add(RoomData);
		}
	}

	return Result;
}
