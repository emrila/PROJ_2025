// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SelectableItemData.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "SelectableContainerData.generated.h"


USTRUCT()
struct FSelectablesContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSelectableItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FSelectableItem, FSelectablesContainer>(Items, DeltaParms, *this);
	}

	void AddOrUpdate(UObject* Selectable, const FVector& Location = FVector::ZeroVector, const bool bCheckLock = true);
	void Remove(const UObject* Selectable);
	void CleanUpInvalidSelectables();
	void SortByLocation();

	bool HasPendingSelectionNotifications(const UObject* Selectable = nullptr) const;
	void ClearPendingNotifications();
	void SetPendingSelectionNotification(const UObject* Selectable = nullptr);

	void ClearAllValidationFlags(const UObject* Selectable = nullptr);
	void ClearPendingValidationNotification(const UObject* Selectable = nullptr);

	bool HasAnyLock(const UObject* Selectable = nullptr) const;
	bool HasAnyConflict(const UObject* Selectable = nullptr) const;

	//------- WAITING -------
	bool HasAnyWaiting(const UObject* Selectable = nullptr) const;
	bool HasWaiting(FGameplayTag Tag, const UObject* Selectable = nullptr) const;
	void SetWaiting(FGameplayTag Tag, const UObject* Selectable = nullptr);

	//------- PENDING -------
	bool HasAnyPending(const UObject* Selectable = nullptr) const;
	bool HasPending(FGameplayTag Tag, const UObject* Selectable = nullptr) const;
	void SetPending(FGameplayTag Tag, const UObject* Selectable = nullptr);

	//------- NOTIFIED -------
	bool HasAnyNotified(const UObject* Selectable = nullptr) const;
	bool HasNotified(FGameplayTag Tag, const UObject* Selectable = nullptr) const;
	void SetNotified(FGameplayTag Tag, const UObject* Selectable = nullptr);

	//------- COMPLETED -------
	bool HasAnyCompleted(const UObject* Selectable = nullptr) const;
	bool HasCompleted(FGameplayTag Tag, const UObject* Selectable = nullptr) const;
	void SetCompleted(FGameplayTag Tag, const UObject* Selectable = nullptr);

protected:
	void Set(FGameplayTag Tag, uint8 Flags, const UObject* Selectable = nullptr);
	bool Has(FGameplayTag Tag, uint8 Flags, const UObject* Selectable = nullptr) const;
	bool HasAny(uint8 Flags, const UObject* Selectable = nullptr) const;
	void MarkAs(const TFunction<void(FSelectableItem&)>& SetState, const UObject* Selectable = nullptr);
	bool ContainsState(const TFunction<bool(const FSelectableItem&)>& HasState, const UObject* Selectable = nullptr) const;
};

template <>
struct TStructOpsTypeTraits<FSelectablesContainer> : public TStructOpsTypeTraitsBase2<FSelectablesContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
