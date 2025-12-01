// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomModifierBase.h"
#include "TimerUserWidget.h"
#include "ParkourTimerMod.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UParkourTimerMod : public URoomModifierBase
{
	GENERATED_BODY()

	virtual void OnRoomEntered(ARoomManagerBase* InRoomManager) override;

	

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_PlayersThatMadeIt)
	TArray<APlayerState*> PlayersThatMadeIt;

	UPROPERTY()
	UTimerUserWidget* TimerWidget;

	UFUNCTION()
	void OnRep_PlayersThatMadeIt() const;

	virtual void BeginReplication() override;
	
	void Multicast_AddTimerWidget();

	UFUNCTION()
	void SetupAfterRoomEntered(ARoomManagerBase* InRoomManager);
	
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
