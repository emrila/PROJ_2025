
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacterBase.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UAttackComponentBase;

DECLARE_LOG_CATEGORY_EXTERN(PlayerBaseLog, Log, All);

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PROJ_2025_API APlayerCharacterBase : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	APlayerCharacterBase();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UAttackComponentBase* GetFirstAttackComponent() const;

	UAttackComponentBase* GetSecondAttackComponent() const;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	
	//Input
	virtual void Move(const FInputActionValue& Value);

	virtual void Look(const FInputActionValue& Value);

	virtual void UseFirstAttackComponent();

	virtual void UseSecondAttackComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* FirstAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* SecondAttackAction;

	UPROPERTY()
	UAttackComponentBase* FirstAttackComponent;

	UPROPERTY()
	UAttackComponentBase* SecondAttackComponent;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
};
