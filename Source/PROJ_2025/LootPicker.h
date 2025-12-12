#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.h"

class PROJ_2025_API FLootPicker
{
public:
	static void Initialize(UDataTable* InDataTable);


	static const FItemDataRow* PickLoot();

private:
	static void InitPools();

	static UDataTable* ItemDataTable;

	static TArray<const FItemDataRow*> CommonPool;
	static TArray<const FItemDataRow*> RarePool;
	static TArray<const FItemDataRow*> LegendaryPool;
};