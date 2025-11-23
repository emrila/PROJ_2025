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
		SetIsReplicatedByDefault(true);
	}
	
	virtual void OnRoomEntered(ARoomManagerBase* InRoomManager)
	{
		RoomManager = InRoomManager;
	}
	
	virtual ~URoomModifierBase() override{}

	int ModLevel = 1;

	UPROPERTY()
	ARoomManagerBase* RoomManager;
};
