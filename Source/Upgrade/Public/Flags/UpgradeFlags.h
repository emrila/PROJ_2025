// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "UpgradeFlags.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EUpgradeFlag : uint8
{
	None = 0 UMETA(DisplayName = "None", Hidden),
	Solo = 1 << 0 UMETA(DisplayName = "Solo"),
	Team = 1 << 1 UMETA(DisplayName = "Team"),
};
ENUM_CLASS_FLAGS(EUpgradeFlag)
