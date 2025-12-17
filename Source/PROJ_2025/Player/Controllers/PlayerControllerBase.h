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
	
	UFUNCTION(BlueprintPure)
	float GetMouseSensitivity() const
	{
		return MouseSensitivity;
	}
	
	UFUNCTION(BlueprintCallable)
	void SetMouseSensitivity(const float NewSensitivity)
	{
		MouseSensitivity = NewSensitivity;
	}
	
	UFUNCTION(Server, Reliable)
	void Server_SetLanPlayerName(const FString& NewName);

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Custom", Getter, Setter)
	float MouseSensitivity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* PlayerBaseMappingContext;
	
};
