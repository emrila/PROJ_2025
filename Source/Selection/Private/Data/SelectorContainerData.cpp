// 🐲Furkan approves of this🐲


#include "Data/SelectorContainerData.h"

#include "Utility/SelectionLog.h"


void FPlayerSelectionContainer::CleanUpInvalidSelections()
{
	const int32 InitialCount = Items.Num();
	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (!Items[i].Selector || !Items[i].Selectable)
		{
			Items.RemoveAt(i);
		}
	}
	if (Items.Num() != InitialCount)
	{
		SELECTION_ERROR(TEXT("Cleaned up %d invalid selections. New count: %d"), InitialCount-Items.Num(), Items.Num());
		MarkArrayDirty();
	}
}

void FPlayerSelectionContainer::AddOrUpdate(const UObject* Selector, const UObject* Selectable)
{
	for (auto& Item : Items)
	{
		if (Item.Selector == Selector)
		{
			SELECTION_DISPLAY(TEXT("Updating selection for selector: %s"), *Selector->GetName());
			Item.Selectable = const_cast<UObject*>(Selectable);
			Item.State = ESelectionDataState::PendingNotify;
			MarkItemDirty(Item);
			return;
		}
	}

	SELECTION_DISPLAY(TEXT("Adding selection for selector: %s"), *Selector->GetName());
	auto& NewItem = Items.AddDefaulted_GetRef();
	NewItem.Selector = const_cast<UObject*>(Selector);
	NewItem.Selectable = const_cast<UObject*>(Selectable);
	NewItem.State = ESelectionDataState::PendingNotify;
	MarkItemDirty(NewItem);
}

void FPlayerSelectionContainer::Remove(const UObject* Selector)
{
	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (Items[i].Selector == Selector)
		{
			SELECTION_DISPLAY(TEXT("Removing selection for selector: %s"), *Selector->GetName());
			Items.RemoveAt(i);
			MarkArrayDirty();
		}
		else
		{
			Items[i].State = ESelectionDataState::PendingNotify;
			MarkItemDirty(Items[i]);
		}
	}
}

UObject* FPlayerSelectionContainer::GetSelectable(const UObject* Selector) const
{
	for (const auto& Item : Items)
	{
		if (Item.Selector == Selector)
		{
			return Item.Selectable;
		}
	}
	return nullptr;
}

UObject* FPlayerSelectionContainer::GetSelector(const UObject* Selectable) const
{
	for (const auto& Item : Items)
	{
		if (Item.Selectable == Selectable)
		{
			return Item.Selector;
		}
	}
	return nullptr;
}

TArray<UObject*> FPlayerSelectionContainer::GetSelectablesSelectors(const UObject* Selectable) const
{
	TArray<UObject*> Selectors;
	for (const auto& Item : Items)
	{
		if (Item.Selectable == Selectable)
		{
			Selectors.Add(Item.Selector);
		}
	}
	return Selectors;
}

TArray<UObject*> FPlayerSelectionContainer::GetSelectorsSelectables(const UObject* Selector) const
{
	TArray<UObject*> Selectables;
	for (const auto& Item : Items)
	{
		if (Item.Selector == Selector)
		{
			Selectables.Add(Item.Selectable);
		}
	}
	return Selectables;
}

TArray<UObject*> FPlayerSelectionContainer::GetSelectablesWithMultipleSelectors(const int32 Threshold) const
{
	TArray<UObject*> Selectables;
	TMap<UObject*, int32> SelectionCountMap;
	for (const auto& Item : Items)
	{
		if (Item.Selectable)
		{
			SelectionCountMap.FindOrAdd(Item.Selectable)++;
		}
	}
	for (const auto& Pair : SelectionCountMap)
	{
		if (Pair.Value > Threshold)
		{
			Selectables.Add(Pair.Key);
		}
	}
	return Selectables;
}

bool FPlayerSelectionContainer::IsCurrentSelectionSelectedBySelector(const UObject* Selector, const UObject* Selectable) const
{
	if (!Selector || !Selectable)
	{
		return false;
	}
	const auto CurrentSelection = GetSelectable(Selector);
	return CurrentSelection && CurrentSelection == Selectable;
}

bool FPlayerSelectionContainer::HasPendingNotifications() const
{
	for (const auto& Item : Items)
	{
		if (Item.State == ESelectionDataState::PendingNotify)
		{
			return true;
		}
	}
	return false;
}

void FPlayerSelectionContainer::ClearPendingNotifications()
{
	MarkAllAs(ESelectionDataState::PendingNotify, ESelectionDataState::Notified);
}

void FPlayerSelectionContainer::MarkAllAs(const ESelectionDataState FromState, const ESelectionDataState ToState)
{
	for (auto& Item : Items)
	{
		if (Item.State == FromState)
		{
			Item.State = ToState;
			MarkItemDirty(Item);
		}
	}
}
