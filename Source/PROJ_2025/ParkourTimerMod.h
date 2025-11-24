// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomModifierBase.h"
#include "ParkourTimerMod.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UParkourTimerMod : public URoomModifierBase
{
	GENERATED_BODY()

	virtual void OnRoomEntered(ARoomManagerBase* InRoomManager) override;

	float Timer;
};
