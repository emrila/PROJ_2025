#include "PlayerCharacterBase.h"


APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAttackComponent* APlayerCharacterBase::GetFirstAttackComponent() const
{
	return nullptr;
}

