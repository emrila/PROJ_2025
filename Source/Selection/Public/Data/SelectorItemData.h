// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Flags/SelectionNotificationFlags.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "SelectorItemData.generated.h"

USTRUCT()
struct FPlayerSelection : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* Selectable = nullptr;

	UPROPERTY()
	UObject* Selector = nullptr;

	UPROPERTY()
	ESelectionDataState State = ESelectionDataState::None;
};
