#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"


class URoomModifierBase;
class UInputMappingContext;
class APlayerCharacterBase;

UCLASS()
class PROJ_2025_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerControllerBase();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void Client_SetSpawnRotation(const FRotator& NewRot);

	UFUNCTION(Server, Reliable)
	void Server_RegisterModifierClient(URoomModifierBase* Modifier);

	UFUNCTION(Server, Reliable)
	void AddDamageTaken(float Damage);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDamageTaken();

	UFUNCTION(Server, Reliable)
	void AddDamageDealt(float Damage);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDamageDealt();
	
protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(Replicated)
	float DamageTaken = 0.f;

	UPROPERTY(Replicated)
	float DamageDealt = 0.f;
	
	UPROPERTY(Replicated)
	APlayerCharacterBase* ControlledPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* PlayerBaseMappingContext;
	
};
