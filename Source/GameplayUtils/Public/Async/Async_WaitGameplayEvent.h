// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync_WaitGameplayEvent.h"
#include "StructUtils/InstancedStruct.h"

#include "Async_WaitGameplayEvent.generated.h"

/**
 * Must be saved as a member variable (and make it UPROPERTY) to avoid GC.
 * Activate() must be manually called (but happens automatically in BP).
 * This which is why we use a subclass of AbilityAsync_WaitGameplayEvent, to expose Activate() so it can be used from C++.
 */
UCLASS()
class GAMEPLAYUTILS_API UAsync_WaitGameplayEvent : public UAbilityAsync_WaitGameplayEvent
{
	GENERATED_BODY()

public:
	UFUNCTION(/*BlueprintCallable, meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE")*/)
	static UAsync_WaitGameplayEvent* CustomWaitGameplayEventToActor(AActor* TargetActor, FGameplayTag EventTag, bool OnlyTriggerOnce = false, bool OnlyMatchExact = true);

	UFUNCTION(/*BlueprintCallable, meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE")*/)
	static UAsync_WaitGameplayEvent* ActivateAndWaitGameplayEventToActor(AActor* TargetActor, FGameplayTag EventTag, bool OnlyTriggerOnce = false, bool OnlyMatchExact = true);

	virtual void Activate() override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FEventReceivedDelegate_CPP, FInstancedStruct);

	FEventReceivedDelegate_CPP EventReceived_CPP;

	UFUNCTION(/*BlueprintPure*/)
	FGameplayTag GetEventTag() const
	{
		return Tag;
	}

protected:
	virtual void GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload) override;
};
