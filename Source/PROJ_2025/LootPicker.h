#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.h"

class PROJ_2025_API FLootPicker
{
public:
	static void Initialize(UDataTable* InDataTable);


	static FItemDataRow* PickLoot(ELootTier& OutRarity);

private:
	static void InitPools();

	static UDataTable* ItemDataTable;

	static TArray<FItemDataRow*> CommonPool;
	static TArray<FItemDataRow*> RarePool;
	static TArray<FItemDataRow*> LegendaryPool;
};