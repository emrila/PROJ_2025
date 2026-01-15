// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "SelectionNotificationFlags.generated.h"

UENUM(BlueprintType)
enum class ESelectionDataState : uint8
{
	None		    UMETA(DisplayName = "None"),
	Waiting         UMETA(DisplayName = "Waiting"),
	PendingNotify   UMETA(DisplayName = "Pending"),
	Notified        UMETA(DisplayName = "Notified"),
};
