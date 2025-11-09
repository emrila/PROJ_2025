#include "PlayerCharacterMage.h"
#include "Player/Components/AttackComponentBase.h"
#include "Player/Components/MageFirstAttackComp.h"

DEFINE_LOG_CATEGORY(MageLog);

APlayerCharacterMage::APlayerCharacterMage()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacterMage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterMage::BeginPlay()
{
	Super::BeginPlay();

	FirstAttackComponent = FindComponentByClass<UMageFirstAttackComp>();
	
}

void APlayerCharacterMage::UseFirstAttackComponent()
{
	Super::UseFirstAttackComponent();

	UE_LOG(MageLog, Warning, TEXT("APlayerCharacterMage::UseFirstAttackComponent called"));

	if (!FirstAttackComponent)
	{
		UE_LOG(MageLog, Error, TEXT("APlayerCharacterMage::UseFirstAttackComponent, FirstAttackComponent is NULL"));
		return;
	}

	GetFirstAttackComponent()->StartAttack();
}




