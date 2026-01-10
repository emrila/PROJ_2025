// 🐲Furkan approves of this🐲


#include "Timers/TaggedEvaluationTimer.h"

FTaggedEvaluationTimer::FTaggedEvaluationTimer()
{
	Reset();
}

void FTaggedEvaluationTimer::DecreaseRemainingDuration(const float InRate)
{
	RemainingDuration = FMath::Max(0.0f, RemainingDuration - InRate);
}

int32 FTaggedEvaluationTimer::GetRemainingDurationInSeconds() const
{
	return FMath::CeilToInt(RemainingDuration);
}

void FTaggedEvaluationTimer::StartTimer(const UWorld* World, const FTimerDelegate& TimerDelegate, const FGameplayTag InTag, const bool
	bIgnoreExisting)
{
	if (!World)
	{
		return;
	}

	if (!bIgnoreExisting && IsActiveWithTag(World, InTag))
	{
		return;
	}

	Reset(InTag);
	World->GetTimerManager().SetTimer(EvaluationTimerHandle, TimerDelegate, Rate, true);
}

void FTaggedEvaluationTimer::ClearTimer(const UWorld* World, const FGameplayTag InTag)
{
	if (World && World->GetTimerManager().IsTimerActive(EvaluationTimerHandle))
	{
		World->GetTimerManager().ClearTimer(EvaluationTimerHandle);
		Reset(InTag);
	}
}

void FTaggedEvaluationTimer::ClearIfActiveWithTag(const UWorld* World, FGameplayTag InTag)
{
	if (IsActiveWithTag(World, InTag))
	{
		ClearTimer(World, InTag);
	}
}

void FTaggedEvaluationTimer::ClearIfActive(const UWorld* World, FGameplayTag InTag)
{
	if (IsActive(World))
	{
		ClearTimer(World, InTag);
	}
}


bool FTaggedEvaluationTimer::IsActive(const UWorld* World) const
{
	if (World)
	{
		return World->GetTimerManager().IsTimerActive(EvaluationTimerHandle);
	}
	return false;
}

bool FTaggedEvaluationTimer::IsComplete() const
{
	return RemainingDuration <= 0.0f;
}

void FTaggedEvaluationTimer::Reset(const FGameplayTag InTag)
{
	RemainingDuration = BaseDuration;
	EvaluationTag = InTag;
}

bool FTaggedEvaluationTimer::IsActiveWithTag(const UWorld* World, const FGameplayTag InTag) const
{
	if (World)
	{
		return World->GetTimerManager().IsTimerActive(EvaluationTimerHandle) && EvaluationTag == InTag;
	}
	return false;
}
