#include "LootPicker.h"

#include "ItemBase.h"


UDataTable* FLootPicker::ItemDataTable = nullptr;
TArray<FItemDataRow*> FLootPicker::CommonPool;
TArray<FItemDataRow*> FLootPicker::RarePool;
TArray<FItemDataRow*> FLootPicker::LegendaryPool;

void FLootPicker::Initialize(UDataTable* InDataTable)
{
    if (!InDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("FLootPicker::Initialize called with null DataTable"));
        return;
    }

    ItemDataTable = InDataTable;
    InitPools();
}

void FLootPicker::InitPools()
{
    if (!ItemDataTable)
        return;

    CommonPool.Empty();
    RarePool.Empty();
    LegendaryPool.Empty();

    for (const FName& RowName : ItemDataTable->GetRowNames())
    {
        FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("FLootPicker InitPools"));
        if (!Row || !Row->ItemClass)
            continue;
        
        for (const ELootTier Tier : Row->SpawnTiers)
        {
            switch (Tier)
            {
            case ELootTier::Common:    CommonPool.Add(Row); break;
            case ELootTier::Rare:      RarePool.Add(Row); break;
            case ELootTier::Legendary: LegendaryPool.Add(Row); break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("LootPicker: Pools initialized: Common=%d, Rare=%d, Legendary=%d"),
        CommonPool.Num(), RarePool.Num(), LegendaryPool.Num());
}

FItemDataRow* FLootPicker::PickLoot()
{
    if (!ItemDataTable)
        return nullptr;

    const float TierRoll = FMath::FRand();
    TArray<FItemDataRow*>* Pool = nullptr;

    if (TierRoll < 0.7f)
        Pool = &CommonPool;        // 70% common
    else if (TierRoll < 0.95f)
        Pool = &RarePool;          // 25% rare
    else
        Pool = &LegendaryPool;     // 5% legendary

    if (!Pool || Pool->Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("DROP POOL EMPTY"));
        return nullptr;
    }

    const int32 Index = FMath::RandRange(0, Pool->Num() - 1);
    FItemDataRow* Row = (*Pool)[Index];
    
    return Row;
}