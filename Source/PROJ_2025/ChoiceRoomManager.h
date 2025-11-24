// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "VotingBooth.h"
#include "ChoiceRoomManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API AChoiceRoomManager : public ARoomManagerBase
{
	GENERATED_BODY()


	virtual void OnRoomInitialized(const FRoomInstance& Room) override;

	virtual void EnableExits() override;

	UPROPERTY()
	AVotingBooth* Booth;
};
