// 🐲Furkan approves of this🐲


#include "Async/Async_WaitForAnimationAsset.h"

UAsync_WaitForAnimationAsset* UAsync_WaitForAnimationAsset::WaitForAnimationAsset(USkeletalMeshComponent* MeshComp, UAnimationAsset* AnimationAsset, const bool bReverse)
{
	UAsync_WaitForAnimationAsset* AsyncTask = NewObject<UAsync_WaitForAnimationAsset>();
	AsyncTask->SkeletalMeshComp = MeshComp;
	AsyncTask->AnimAsset = AnimationAsset;
	AsyncTask->bReverse = bReverse;

	return AsyncTask;
}

void UAsync_WaitForAnimationAsset::Activate()
{
	Super::Activate();
	if (!SkeletalMeshComp || ! AnimAsset)
	{
		EndAnimationTask();
	}
	SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	AnimDuration = AnimAsset->GetPlayLength();

	if (bReverse)
	{
		SkeletalMeshComp->SetPosition(AnimDuration);
		SkeletalMeshComp->SetPlayRate(-1.f);
	}
	else
	{
		SkeletalMeshComp->SetPosition(0.f);
		SkeletalMeshComp->SetPlayRate(1.f);
	}

	SkeletalMeshComp->PlayAnimation(AnimAsset, false);
	bIsTicking = true;

	if (const UWorld* World = GEngine->GetWorldFromContextObjectChecked(SkeletalMeshComp->GetOwner()))
	{
		RegisterWithGameInstance(World);
	}
	else
	{
		EndAnimationTask();
	}

}

void UAsync_WaitForAnimationAsset::Tick(float DeltaTime)
{
	if (!bIsTicking)
	{
		return;
	}

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= AnimDuration)
	{
		bIsTicking = false;
		OnComplete.Broadcast();
	}
}

void UAsync_WaitForAnimationAsset::EndAnimationTask()
{
	bIsTicking = false;
	OnComplete.Broadcast();
	SetReadyToDestroy();
}
