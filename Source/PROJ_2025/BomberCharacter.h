// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BomberCharacter.generated.h"


UCLASS()
class PROJ_2025_API ABomberCharacter : public AEnemyBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	ABomberCharacter();

protected:
	virtual void HandleDeath() override;

public:	

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsDiving = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsExploding = false;

	UFUNCTION(Server, Reliable)
	void Server_SpawnExplosion(FVector SpawnLocation, FRotator SpawnRotation);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_Explode();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ExplosionActor;
	
	virtual void HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser) override;

};
