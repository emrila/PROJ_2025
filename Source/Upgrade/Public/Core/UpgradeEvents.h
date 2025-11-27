#pragma once

#include "UpgradeDisplayData.h"
#include "UpgradeEvents.generated.h"

using FUpgradeData = FUpgradeDisplayData;

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgrade, FUpgradeDisplayData, SelectedUpgrade);

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeEvent);

