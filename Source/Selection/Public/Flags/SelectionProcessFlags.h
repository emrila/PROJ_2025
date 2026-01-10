// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "SelectionProcessFlags.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESelectionEvaluationFlag : uint8
{
	None = 0 UMETA(Hidden),

	//Process Flags
	Waiting = 1 << 0 UMETA(DisplayName = "Waiting"),
	Processing = 1 << 1 UMETA(DisplayName = "Processing"),
	Complete = 1 << 2 UMETA(DisplayName = "Complete"),

	//Notification Flags
	PendingNotify = 1 << 3 UMETA(DisplayName = "Pending"),
	Notified = 1 << 4 UMETA(DisplayName = "Notified"),

	//Evaluation Flags
	Lock = 1 << 5 UMETA(DisplayName = "Lock"),
	Conflict = 1 << 6 UMETA(DisplayName = "Conflict"),
};
ENUM_CLASS_FLAGS(ESelectionEvaluationFlag)

// For convenient uint8 conversion when working with UPROPERTY flags
//----- SELECTION EVALUATION FLAG MACROS -----
#define WAITING_FLAG static_cast<uint8>(ESelectionEvaluationFlag::Waiting)
#define PENDING_NOTIFY_FLAG static_cast<uint8>(ESelectionEvaluationFlag::PendingNotify)
#define NOTIFIED_FLAG static_cast<uint8>(ESelectionEvaluationFlag::Notified)
#define COMPLETE_FLAG static_cast<uint8>(ESelectionEvaluationFlag::Complete)

#define LOCK_FLAG static_cast<uint8>(ESelectionEvaluationFlag::Lock)
#define CONFLICT_FLAG static_cast<uint8>(ESelectionEvaluationFlag::Conflict)
//----- END SELECTION EVALUATION FLAG MACROS -----

//----- COMBINATION MACROS -----
#define WAITING_CONFLICT_FLAGS static_cast<uint8>(WAITING_FLAG | CONFLICT_FLAG)
#define PENDING_CONFLICT_FLAGS static_cast<uint8>(PENDING_NOTIFY_FLAG | CONFLICT_FLAG)
#define NOTIFIED_CONFLICT_FLAGS static_cast<uint8>(NOTIFIED_FLAG | CONFLICT_FLAG)
#define COMPLETE_CONFLICT_FLAGS static_cast<uint8>(COMPLETE_FLAG | CONFLICT_FLAG)

#define WAITING_LOCK_FLAGS static_cast<uint8>(WAITING_FLAG | LOCK_FLAG)
#define PENDING_LOCK_FLAGS static_cast<uint8>(PENDING_NOTIFY_FLAG | LOCK_FLAG)
#define NOTIFIED_LOCK_FLAGS static_cast<uint8>(NOTIFIED_FLAG | LOCK_FLAG)
#define COMPLETE_LOCK_FLAGS static_cast<uint8>(COMPLETE_FLAG | LOCK_FLAG)

#define EVALUATION_FLAGS static_cast<uint8>(WAITING_FLAG | PENDING_NOTIFY_FLAG | NOTIFIED_FLAG)
//----- END COMBINATION MACROS -----
