// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "SelectorItemData.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "SelectorContainerData.generated.h"


USTRUCT()
struct FPlayerSelectionContainer : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FPlayerSelection> Items;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FastArrayDeltaSerialize<FPlayerSelection, FPlayerSelectionContainer>(Items, DeltaParms, *this);
    }
    void CleanUpInvalidSelections();

    void AddOrUpdate(const UObject* Selector, const UObject* Selectable);
    void Remove(const UObject* Selector);

    UObject* GetSelectable(const UObject* Selector) const;
    UObject* GetSelector(const UObject* Selectable) const;

    TArray<UObject*> GetSelectablesSelectors(const UObject* Selectable) const;
    TArray<UObject*> GetSelectorsSelectables(const UObject* Selector) const;
    TArray<UObject*> GetSelectablesWithMultipleSelectors(const int32 Threshold) const;

    bool IsCurrentSelectionSelectedBySelector(const UObject* Selector, const UObject* Selectable) const;

    bool HasPendingNotifications() const;
    void ClearPendingNotifications();

protected:
    void MarkAllAs(ESelectionDataState FromState, ESelectionDataState ToState);
};

template<>
struct TStructOpsTypeTraits<FPlayerSelectionContainer> : public TStructOpsTypeTraitsBase2<FPlayerSelectionContainer>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};
