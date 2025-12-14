#include "LootPicker.h"

#include "ItemBase.h"


UDataTable* FLootPicker::ItemDataTable = nullptr;
TArray<FName> FLootPicker::CommonPool;
TArray<FName> FLootPicker::RarePool;
TArray<FName> FLootPicker::LegendaryPool;

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
        
        switch (Row->LootTier)
            {
            case ELootTier::Common:    CommonPool.Add(RowName); break;
            case ELootTier::Rare:      RarePool.Add(RowName); break;
            case ELootTier::Legendary: LegendaryPool.Add(RowName); break;
            }
    }
}

FName FLootPicker::PickLoot()
{
    if (!ItemDataTable)
        return FName();

    const float TierRoll = FMath::FRand();
    TArray<FName>* Pool = nullptr;

    if (TierRoll < 0.7f) // 70%common
    {
        Pool = &CommonPool;
    }
    else if (TierRoll < 0.95f) // 25%rare
    {
        Pool = &RarePool;
    }
    else // 5% legendary
    {
        Pool = &LegendaryPool;
    }   

    if (!Pool || Pool->Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("DROP POOL EMPTY"));
        return FName();
    }

    const int32 Index = FMath::RandRange(0, Pool->Num() - 1);
    FName RandomRow = (*Pool)[Index];
    
    return RandomRow;
}

FItemDataRow FLootPicker::GetItem(const FName RowName)
{
    return *ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("FLootPicker InitPools"));
}
