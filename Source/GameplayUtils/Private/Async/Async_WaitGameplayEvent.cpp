// 🐲Furkan approves of this🐲


#include "Async/Async_WaitGameplayEvent.h"

UAsync_WaitGameplayEvent* UAsync_WaitGameplayEvent::CustomWaitGameplayEventToActor(AActor* TargetActor, FGameplayTag EventTag, bool OnlyTriggerOnce, bool OnlyMatchExact)
{
	UAsync_WaitGameplayEvent* MyObj = NewObject<UAsync_WaitGameplayEvent>();
	MyObj->SetAbilityActor(TargetActor);
	MyObj->Tag = EventTag;
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;
	MyObj->OnlyMatchExact = OnlyMatchExact;
	return MyObj;
}

UAsync_WaitGameplayEvent* UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(AActor* TargetActor, FGameplayTag EventTag, bool OnlyTriggerOnce, bool OnlyMatchExact)
{
	UAsync_WaitGameplayEvent* MyObj = CustomWaitGameplayEventToActor(TargetActor, EventTag, OnlyTriggerOnce, OnlyMatchExact);
	MyObj->Activate();
	return MyObj;
}

void UAsync_WaitGameplayEvent::Activate()
{
	Super::Activate();
}

void UAsync_WaitGameplayEvent::GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastDelegates() && EventReceived_CPP.IsBound())
	{
		FGameplayEventData TempPayload = *Payload;
		TempPayload.EventTag = MatchingTag;
		EventReceived_CPP.Broadcast(FInstancedStruct::Make<FGameplayEventData>(MoveTemp(TempPayload)));

	}
	Super::GameplayEventContainerCallback(MatchingTag, Payload);
}
