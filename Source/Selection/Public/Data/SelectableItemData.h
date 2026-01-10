// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Flags/SelectionNotificationFlags.h"
#include "Flags/SelectionProcessFlags.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Utility/FlagUtils.h"
#include "SelectableItemData.generated.h"

USTRUCT()
struct FSelectableItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* Selectable = nullptr;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	ESelectionDataState State = ESelectionDataState::None;

	UPROPERTY()
	uint8 EvaluationFlags = 0;

	FORCEINLINE bool HasPendingEvaluationNotification() const
	{
		return HasPendingConflict() || HasPendingLock();
	}

	FORCEINLINE void ClearEvaluationNotification()
	{
		ClearFlag(EvaluationFlags, EVALUATION_FLAGS);
	}

	//------- CONFLICT
	FORCEINLINE bool HasConflictFlag() const
	{
		return HasFlag(EvaluationFlags, CONFLICT_FLAG);
	}

	FORCEINLINE bool HasWaitingConflict() const
	{
		return HasAllFlags(EvaluationFlags, WAITING_CONFLICT_FLAGS);
	}

	FORCEINLINE void SetWaitingConflict(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}
		SetFlag(EvaluationFlags, WAITING_CONFLICT_FLAGS);
	}

	FORCEINLINE bool HasPendingConflict() const
	{
		return HasAllFlags(EvaluationFlags, PENDING_CONFLICT_FLAGS);
	}

	FORCEINLINE void SetPendingConflict(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}
		SetFlag(EvaluationFlags, PENDING_CONFLICT_FLAGS);
	}

	FORCEINLINE bool HasNotifiedConflict() const
	{
		return HasAllFlags(EvaluationFlags, NOTIFIED_CONFLICT_FLAGS);
	}

	FORCEINLINE void SetNotifiedConflict(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}

		SetFlag(EvaluationFlags, NOTIFIED_CONFLICT_FLAGS);
	}

	FORCEINLINE bool HasCompletedConflict() const
	{
		return HasAllFlags(EvaluationFlags, COMPLETE_CONFLICT_FLAGS);
	}

	FORCEINLINE void SetCompletedConflict(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}
		SetFlag(EvaluationFlags, COMPLETE_CONFLICT_FLAGS);
	}

	//------- LOCK
	FORCEINLINE bool HasLockFlag() const
	{
		return HasFlag(EvaluationFlags, LOCK_FLAG);
	}

	FORCEINLINE bool HasWaitingLock() const
	{
		return HasAllFlags(EvaluationFlags, WAITING_LOCK_FLAGS);
	}

	FORCEINLINE void SetWaitingLock(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}

		SetFlag(EvaluationFlags, WAITING_LOCK_FLAGS);
	}

	FORCEINLINE bool HasPendingLock() const
	{
		return HasAllFlags(EvaluationFlags, PENDING_LOCK_FLAGS);
	}

	FORCEINLINE void SetPendingLock(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}
		SetFlag(EvaluationFlags, PENDING_LOCK_FLAGS);
	}

	FORCEINLINE bool HasNotifiedLock() const
	{
		return HasAllFlags(EvaluationFlags, NOTIFIED_LOCK_FLAGS);
	}

	FORCEINLINE void SetNotifiedLock(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}
		SetFlag(EvaluationFlags, NOTIFIED_LOCK_FLAGS);
	}

	FORCEINLINE bool HasCompletedLock() const
	{
		return HasAllFlags(EvaluationFlags, COMPLETE_LOCK_FLAGS);
	}

	FORCEINLINE void SetCompletedLock(const bool bClearExisting = true)
	{
		if (bClearExisting)
		{
			EvaluationFlags = 0;
		}
		SetFlag(EvaluationFlags, COMPLETE_LOCK_FLAGS);
	}
};
