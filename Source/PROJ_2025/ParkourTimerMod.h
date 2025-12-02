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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_PlayersThatMadeIt)
	TArray<APlayerState*> PlayersThatMadeIt;

	UPROPERTY()
	UTimerUserWidget* TimerWidget;
	
	FTimerHandle TimerHandle;
	
	UFUNCTION()
	void OnRep_PlayersThatMadeIt() const;

	virtual void OnAllClientsReady() override;

	virtual void OnExitsUnlocked() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveTimerWidget();

	virtual void BeginReplication() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddTimerWidget(float Timer);

	void DealDamageToPlayers();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FinishTimer();
	
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
