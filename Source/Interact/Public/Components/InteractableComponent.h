// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Interactable.h"
#include "InteractableComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INTERACT_API UInteractableComponent : public UActorComponent, public IInteractable //TODO: make inherit shape or sphere component to have collision
{
	GENERATED_BODY()

public:
	UInteractableComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Server, Reliable)
    void Server_OnInteract(UObject* Interactor);

	UFUNCTION()
	void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	
	UFUNCTION()
	void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UFUNCTION()
	void OnProximityEvent(FGameplayEventData Payload);

	void OnOverlap(AActor* OtherActor, FGameplayTag EventTag) const;

private:
	AActor* GetTargetOwner() const;

protected:
	UPROPERTY()
	TArray<class UAsync_WaitGameplayEvent*> WaitGameplayEvents;

	/* Issue: "doubled up OnPress events for the same keypress under some conditions"
	 * Post: https://forums.unrealengine.com/t/inputcomponent-bindings-sometimes-trigger-twice/2532480/2
	 * Fix in 5.7.1: https://github.com/EpicGames/UnrealEngine/commit/44c2dcbda88c526e08987893e897f091d26781d9*/
	UFUNCTION()
	void SetInteractionInputEnabled(const bool bEnabled)
	{
		bInteractionInputEnabled = bEnabled;
	}
	bool bInteractionInputEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Interaction")
	bool bUseOwnersOwnerAsTarget = true;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag InteractTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag ProximityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag EnterTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag ExitTag;
};
