// 🐲Furkan approves of this🐲


#include "Tasks/AnimAssetCompleteTask.h"
#include "AbilitySystemComponent.h"


UAnimAssetCompleteTask* UAnimAssetCompleteTask::WaitForAnimationAsset(UGameplayAbility* OwningAbility, UAnimationAsset* AnimationAsset)
{
	UAnimAssetCompleteTask* Task = NewAbilityTask<UAnimAssetCompleteTask>(OwningAbility);
	Task->AnimAsset = AnimationAsset;
	return Task;
}
void UAnimAssetCompleteTask::Activate()
{
	Super::Activate();
	if (!AnimAsset)
	{
		EndTask();
		return;
	}
	AnimDuration = AnimAsset->GetPlayLength();
	ElapsedTime = 0.f;
	bIsTicking = true;
}

void UAnimAssetCompleteTask::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (!bIsTicking)
	{
		return;
	}

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= AnimDuration)
	{
		EndAnimationTask();
	}
}

void UAnimAssetCompleteTask::EndAnimationTask()
{
	bIsTicking = false;
	OnComplete.Broadcast();
	EndTask();
}
