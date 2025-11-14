// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardGameInstance.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"

TArray<URoomData*> UWizardGameInstance::GetAllRoomData() const
{
	TArray<URoomData*> Result;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(URoomData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName("/Game/RoomData/Normal")); 
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
URoomData* UWizardGameInstance::GetCampRoomData() const
{
	FAssetRegistryModule& AssetRegistryModule = 
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(URoomData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName("/Game/RoomData/Camp"));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Camp RoomData assets found!"));
		return nullptr;
	}

	return Cast<URoomData>(AssetList[0].GetAsset());
}

bool UWizardGameInstance::RollForCampRoom()
{
	if (ChanceForCamp == 0.f)
	{
		ChanceForCamp = 0.01f;
		return false;
	}
	if (FMath::FRand() <= ChanceForCamp)
	{
		ChanceForCamp = 0.001f;
		return true;
	}
	if (ChanceForCamp == 0.001f)
	{
		ChanceForCamp = 0.01f;
	}
	else if (ChanceForCamp == 0.01f)
	{
		ChanceForCamp = 0.03f;
	}
	else if (ChanceForCamp == 0.03f)
	{
		ChanceForCamp = 0.1f;
	}
	else if (ChanceForCamp == 0.1f)
	{
		ChanceForCamp = 0.5f;
	}
	else if (ChanceForCamp == 0.5f)
	{
		ChanceForCamp = 0.9f;
	}
	else if (ChanceForCamp == 0.9f)
	{
		ChanceForCamp = 1.f;
	}
	return false;
	
}
