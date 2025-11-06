// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"

class AEnemySpawn;



USTRUCT(BlueprintType)
struct FCombatWave
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Wave")
	TArray<AEnemySpawn*> Enemies;
};
UCLASS()
class PROJ_2025_API ACombatManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACombatManager();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_StartWave();

	void StartWave_Internal();


protected:

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	TArray<FCombatWave> Waves;

	
};
