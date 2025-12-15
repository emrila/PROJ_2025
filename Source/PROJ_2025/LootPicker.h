#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.h"

class PROJ_2025_API FLootPicker
{
public:
	static void Initialize(UDataTable* InDataTable);


	static FName PickLoot();

	static FItemDataRow GetItem(const FName RowName);

private:
	static void InitPools();

	static UDataTable* ItemDataTable;

	static TArray<FName> CommonPool;
	static TArray<FName> RarePool;
	static TArray<FName> LegendaryPool;
};