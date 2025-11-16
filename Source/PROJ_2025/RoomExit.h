// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomExit.generated.h"

class URoomData;

UCLASS()
class PROJ_2025_API ARoomExit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomExit();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_LinkedRoomData)
	URoomData* LinkedRoomData;

	UFUNCTION(BlueprintImplementableEvent)
	void EnableExit();

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool CanExit = false;
	
	UFUNCTION()
	void OnRep_LinkedRoomData() { OnRoomLinked(); }

	UFUNCTION(BlueprintImplementableEvent)
	void OnRoomLinked();
	//UPROPERTY(BlueprintReadOnly)
	//FRoomModifier LinkedModifier; 


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
