// 🐲Furkan approves of this🐲


#include "Abilities/PlaySingleAnimationAbility.h"
#include <Animation/AnimationAsset.h>
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/SkeletalMeshComponent.h"

void UPlaySingleAnimationAbility::PlayAnimationAssetReverse(USkeletalMeshComponent* MeshComp, UAnimationAsset* AnimationAsset, bool bLoop)
{
	if (MeshComp && AnimationAsset)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComp->PlayAnimation(AnimationAsset, bLoop);

		if (UAnimSingleNodeInstance* AnimInstance = MeshComp->GetSingleNodeInstance())
		{
			AnimInstance->SetReverse(true);
		}
	}
}

void UPlaySingleAnimationAbility::PlayAnimationAsset(USkeletalMeshComponent* MeshComp, UAnimationAsset* AnimationAsset, bool bLoop)
{
	if (MeshComp && AnimationAsset)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComp->PlayAnimation(AnimationAsset, bLoop);
	}
}
