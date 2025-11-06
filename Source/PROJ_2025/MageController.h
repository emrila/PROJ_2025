
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MageController.generated.h"

class UInputMappingContext;
struct FInputActionValue;
class ACharacter;
class UCharacterMovementComponent;
class UInputAction;

UCLASS()
class PROJ_2025_API AMageController : public APlayerController
{
	GENERATED_BODY()

public:

	AMageController();

	virtual void Tick(float DeltaSeconds) override;

	virtual void Move(const FInputActionValue& Value);

	virtual void Look(const FInputActionValue& Value);
protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupInputComponent() override;

	virtual void DoMove(float Right, float Forward);

	virtual void DoLook(float Yaw, float Pitch);

	virtual void DoJumpStart();

	virtual void DoJumpEnd();

	virtual void DoShoot();


	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;

	UPROPERTY()
	ACharacter* ControlledCharacter;


	//Input
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputMappingContext* MageMappingContext;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
};
