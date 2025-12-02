// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "Components/ActorComponent.h"
#include "RoomModifierBase.generated.h"

/**
 * 
 */

class URoomModifierBase;
class ARoomManagerBase;

USTRUCT(BlueprintType)
struct FRoomModifierArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<URoomModifierBase>> Modifiers;
};

UCLASS(Abstract, ClassGroup=(RoomModifiers))
class PROJ_2025_API URoomModifierBase : public UActorComponent
{
	GENERATED_BODY()

public:
	URoomModifierBase()
	{
		PrimaryComponentTick.bCanEverTick = true;
		SetIsReplicated(true);
	}
	
	virtual void OnRoomEntered(ARoomManagerBase* InRoomManager);

	virtual void OnExitsUnlocked();

	UFUNCTION(Server, Reliable)
	void RegisterClient();

	virtual void OnAllClientsReady();
	
	int ClientsReady = 0;

	int ModLevel = 1;

	UPROPERTY()
	ARoomManagerBase* RoomManager;
};
