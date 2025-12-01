// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomModifierBase.h"
#include "MoreEnemiesMod.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UMoreEnemiesMod : public URoomModifierBase
{
	GENERATED_BODY()


	virtual void OnRoomEntered(ARoomManagerBase* InRoomManager) override;
};
