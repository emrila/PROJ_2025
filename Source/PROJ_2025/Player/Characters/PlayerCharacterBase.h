
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacterBase.generated.h"

class UInteractorComponent;
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
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UAttackComponentBase* GetFirstAttackComponent() const;

	UAttackComponentBase* GetSecondAttackComponent() const;

	FVector GetRightHandSocketLocation() const;

	FVector GetLeftHandSocketLocation() const;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,AActor* DamageCauser) override;
	
	//Input
	virtual void Move(const FInputActionValue& Value);

	virtual void Look(const FInputActionValue& Value);

	virtual void UseFirstAttackComponent();

	virtual void UseSecondAttackComponent();

	virtual void Interact(const FInputActionValue& Value);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Ability")
	UInputAction* FirstAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Ability")
	UInputAction* SecondAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input|Misc")
	UInputAction* InteractAction;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Ability")
	UAttackComponentBase* FirstAttackComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Ability")
	UAttackComponentBase* SecondAttackComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Misc")
	TObjectPtr<UInteractorComponent> InteractorComponent;
	//Socket Names
	UPROPERTY(VisibleAnywhere, Category="Socket Names")
	FName RightHandSocket = TEXT("HandGrip_R");

	UPROPERTY(VisibleAnywhere, Category="Socket Names")
	FName LeftHandSocket = TEXT("HandGrip_L");
	
};
