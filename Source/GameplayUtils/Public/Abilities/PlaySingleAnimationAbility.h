// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PlaySingleAnimationAbility.generated.h"

UCLASS()
class GAMEPLAYUTILS_API UPlaySingleAnimationAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void PlayAnimationAssetReverse(USkeletalMeshComponent* MeshComp, UAnimationAsset* AnimationAsset, bool bLoop);

	UFUNCTION(BlueprintCallable)
	void PlayAnimationAsset(USkeletalMeshComponent* MeshComp, UAnimationAsset* AnimationAsset, bool bLoop);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true) )
	TObjectPtr<UAnimationAsset> Animation;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true) )
	USkeletalMeshComponent* SkeletalMeshComponent;
};
