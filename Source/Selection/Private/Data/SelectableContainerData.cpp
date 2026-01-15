// 🐲Furkan approves of this🐲


#include "Data/SelectableContainerData.h"

#include "Utility/SelectionLog.h"
#include "Utility/Tags.h"

void FSelectablesContainer::AddOrUpdate(UObject* Selectable, const FVector& Location, const bool bCheckLock)
{
	if (!Selectable)
	{
		SELECTION_ERROR(TEXT("FSelectablesContainer::AddOrUpdate called with null selectable."));
		return;
	}
	SELECTION_DISPLAY(TEXT("AddOrUpdate Selectable: %s"), *Selectable->GetName());
	for (auto& Item : Items)
	{
		if (Item.Selectable == Selectable)
		{
			if (bCheckLock && Item.HasNotifiedLock())
			{
				return;
			}

			if (Item.Location != Location)
			{
				SELECTION_DISPLAY(TEXT("Updating location for selectable: %s"), *Selectable->GetName());
				Item.Location = Location;
				Item.State = ESelectionDataState::PendingNotify;
				Item.EvaluationFlags = 0;
				MarkItemDirty(Item);
			}
			return;
		}
	}
	SELECTION_DISPLAY(TEXT("Adding selectable: %s"), *Selectable->GetName());

	auto& NewItem = Items.AddDefaulted_GetRef();
	NewItem.Selectable = Selectable;
	NewItem.Location = Location;
	NewItem.State = ESelectionDataState::PendingNotify;
	NewItem.EvaluationFlags = 0;
	MarkItemDirty(NewItem);
}

void FSelectablesContainer::Remove(const UObject* Selectable)
{
	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (Items[i].Selectable == Selectable)
		{
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

void FSelectablesContainer::CleanUpInvalidSelectables()
{
	const int32 InitialCount = Items.Num();
	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (!Items[i].Selectable)
		{
			Items.RemoveAt(i);
		}
	}
	if (Items.Num() != InitialCount)
	{
		SELECTION_DISPLAY(TEXT("Cleaned up %d invalid selectables. New count: %d"), InitialCount - Items.Num(), Items.Num());
		MarkArrayDirty();
	}
}

void FSelectablesContainer::SortByLocation()
{
	SELECTION_DISPLAY(TEXT("Sorting selectables by location."));
	Items.Sort([](const FSelectableItem& A, const FSelectableItem& B)
	{
		return A.Location.SizeSquared() < B.Location.SizeSquared();
	});

	MarkAs([&](FSelectableItem& Item)
	{
		Item.State = ESelectionDataState::PendingNotify;
		MarkItemDirty(Item);
	});
}

//------- Selection Notifications -------

bool FSelectablesContainer::HasPendingSelectionNotifications(const UObject* Selectable) const
{
	return ContainsState([&](const FSelectableItem& Item)
	{
		return Item.State == ESelectionDataState::PendingNotify;
	}, Selectable);
}

void FSelectablesContainer::ClearPendingNotifications()
{
	MarkAs([&](FSelectableItem& Item)
	{
		if (Item.State == ESelectionDataState::PendingNotify)
		{
			Item.State = ESelectionDataState::Notified;
		}

		MarkItemDirty(Item);
	});
}

void FSelectablesContainer::SetPendingSelectionNotification(const UObject* Selectable)
{
	MarkAs([&](FSelectableItem& Item)
	{
		Item.State = ESelectionDataState::PendingNotify;
		MarkItemDirty(Item);
	}, Selectable);
}

//------- Validation Flags -------

void FSelectablesContainer::ClearAllValidationFlags(const UObject* Selectable)
{
	MarkAs([&](FSelectableItem& Item)
	{
		Item.EvaluationFlags = 0;
		MarkItemDirty(Item);
	}, Selectable);
}

void FSelectablesContainer::ClearPendingValidationNotification(const UObject* Selectable)
{
	MarkAs([&](FSelectableItem& Item)
	{
		Item.ClearEvaluationNotification();
		MarkItemDirty(Item);
	}, Selectable);
}


bool FSelectablesContainer::HasAnyLock(const UObject* Selectable) const
{
	return ContainsState([&](const FSelectableItem& Item)
	{
		return Item.HasWaitingLock() || Item.HasPendingLock() || Item.HasNotifiedLock() || Item.HasCompletedLock();
	}, Selectable);
}

bool FSelectablesContainer::HasAnyConflict(const UObject* Selectable) const
{
	return ContainsState([&](const FSelectableItem& Item)
	{
		return Item.HasWaitingConflict() || Item.HasPendingConflict() || Item.HasNotifiedConflict() || Item.HasCompletedConflict();
	}, Selectable);
}

//------- WAITING -------
bool FSelectablesContainer::HasAnyWaiting(const UObject* Selectable) const
{
	return HasAny(WAITING_FLAG, Selectable);
}

bool FSelectablesContainer::HasWaiting(const FGameplayTag Tag, const UObject* Selectable) const
{
	return Has(Tag, WAITING_FLAG, Selectable);
}

void FSelectablesContainer::SetWaiting(const FGameplayTag Tag, const UObject* Selectable)
{
	Set(Tag, WAITING_FLAG, Selectable);
}

//------- PENDING -------
bool FSelectablesContainer::HasAnyPending(const UObject* Selectable) const
{
	return HasAny(PENDING_NOTIFY_FLAG, Selectable);
}

bool FSelectablesContainer::HasPending(const FGameplayTag Tag, const UObject* Selectable) const
{
	return Has(Tag, PENDING_NOTIFY_FLAG, Selectable);
}

void FSelectablesContainer::SetPending(const FGameplayTag Tag, const UObject* Selectable)
{
	Set(Tag, PENDING_NOTIFY_FLAG, Selectable);
}

//------- NOTIFIED -------
bool FSelectablesContainer::HasAnyNotified(const UObject* Selectable) const
{
	return HasAny(NOTIFIED_FLAG, Selectable);
}

bool FSelectablesContainer::HasNotified(const FGameplayTag Tag, const UObject* Selectable) const
{
	return Has(Tag, NOTIFIED_FLAG, Selectable);
}

void FSelectablesContainer::SetNotified(const FGameplayTag Tag, const UObject* Selectable)
{
	Set(Tag, NOTIFIED_FLAG, Selectable);
}

//------- COMPLETED -------
bool FSelectablesContainer::HasAnyCompleted(const UObject* Selectable) const
{
	return HasAny(COMPLETE_FLAG, Selectable);
}

bool FSelectablesContainer::HasCompleted(const FGameplayTag Tag, const UObject* Selectable) const
{
	return Has(Tag, COMPLETE_FLAG, Selectable);
}

void FSelectablesContainer::SetCompleted(const FGameplayTag Tag, const UObject* Selectable)
{
	Set(Tag, COMPLETE_FLAG, Selectable);
}

//------- INTERNALS -------
void FSelectablesContainer::Set(const FGameplayTag Tag, const uint8 Flags, const UObject* Selectable)
{
	uint8 ValidationFlags;
	if (Tag.MatchesTag(EVENT_TAG_VALIDATION_LOCK))
	{
		ValidationFlags = static_cast<uint8>(Flags | LOCK_FLAG);
	}
	else if (Tag.MatchesTag(EVENT_TAG_VALIDATION_CONFLICT))
	{
		ValidationFlags = static_cast<uint8>(Flags | CONFLICT_FLAG);
	}
	else
	{
		SELECTION_ERROR(TEXT("%hs: Unknown tag %s for checking flags."), __FUNCTION__, *Tag.ToString());
		return;
	}

	MarkAs([&](FSelectableItem& Item)
	{
		Item.EvaluationFlags = 0;
		SetFlag(Item.EvaluationFlags, ValidationFlags);
		MarkItemDirty(Item);
	}, Selectable);
}

bool FSelectablesContainer::Has(const FGameplayTag Tag, const uint8 Flags, const UObject* Selectable) const
{
	uint8 ValidationFlags;
	if (Tag.MatchesTag(EVENT_TAG_VALIDATION_LOCK))
	{
		ValidationFlags = static_cast<uint8>(Flags | LOCK_FLAG);
	}
	else if (Tag.MatchesTag(EVENT_TAG_VALIDATION_CONFLICT))
	{
		ValidationFlags = static_cast<uint8>(Flags | CONFLICT_FLAG);
	}
	else
	{
		SELECTION_ERROR(TEXT("%hs: Unknown tag %s for checking flags."), __FUNCTION__, *Tag.ToString());
		return false;
	}
	return ContainsState([&](const FSelectableItem& Item)
		{
			 return HasAllFlags(Item.EvaluationFlags, ValidationFlags);
		}, Selectable);
}

bool FSelectablesContainer::HasAny(const uint8 Flags, const UObject* Selectable) const
{
	return ContainsState([&](const FSelectableItem& Item)
	{
		return HasAllFlags(Item.EvaluationFlags, Flags);
	}, Selectable);
}

void FSelectablesContainer::MarkAs(const TFunction<void(FSelectableItem&)>& SetState, const UObject* Selectable)
{
	if (!Selectable)
	{
		for (auto& Item : Items)
		{
			SetState(Item);
		}
	}
	else
	{
		for (auto& Item : Items)
		{
			if (Item.Selectable == Selectable)
			{
				SetState(Item);
				return;
			}
		}
	}
}

bool FSelectablesContainer::ContainsState(const TFunction<bool(const FSelectableItem&)>& HasState, const UObject* Selectable) const
{
	if (!Selectable)
	{
		for (const auto& Item : Items)
		{
			if (HasState(Item))
			{
				return true;
			}
		}
	}
	else
	{
		for (const auto& Item : Items)
		{
			if (Item.Selectable == Selectable)
			{
				return HasState(Item);
			}
		}
	}
	return false;
}
