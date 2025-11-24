// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "ParkourManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API AParkourManager : public ARoomManagerBase
{
	GENERATED_BODY()
public:
	
	virtual void OnRoomInitialized(const FRoomInstance& Room) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float TimerIfTimeTrial;
	
};
