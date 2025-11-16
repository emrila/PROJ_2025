#pragma once

#include "NiagaraFunctionLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacterBase.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class UInteractorComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UAttackComponentBase;

DECLARE_LOG_CATEGORY_EXTERN(PlayerBaseLog, Log, All);

UCLASS()
class PROJ_2025_API APlayerCharacterBase : public ACharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacterBase();
	
	//Handle override parent functions
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	//Handle components
	UAttackComponentBase* GetFirstAttackComponent() const;

	UAttackComponentBase* GetSecondAttackComponent() const;
	
	//Handle sockets
	FVector GetRightHandSocketLocation() const;

	FVector GetLeftHandSocketLocation() const;
	
	AActor* GetRightHandAttachedActor() const;
	
	//Handle camera
	virtual void HandleCameraDetachment();
	
	virtual void HandleCameraReattachment();
	
	UFUNCTION(Client, Reliable)
	virtual void Client_StartCameraInterpolation(
		const FVector& TargetLocation, const FRotator& TargetRotation, const float LerpDuration);

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//TODO: This may be moved to the belonging attack component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Particles")
	UNiagaraSystem* ImpactParticles;

protected:
	//Handle override parent functions
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,AActor* DamageCauser) override;
	
	//Handle nametag
	virtual void TickNotLocal();
	
	//Handle camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UCameraComponent* FollowCamera;
	
	//Stored when detaching camera
	FVector FollowCameraRelativeLocation = FVector::ZeroVector;
	FRotator FollowCameraRelativeRotation = FRotator::ZeroRotator;
	
	//Handle camera interpolation
	virtual void InterpolateCamera(FTransform& TargetTransform, const float LerpDuration);
	
	UPROPERTY(Transient)
	bool bIsInterpolatingCamera = false;
	
	UPROPERTY(Transient)
	float CameraInterpElapsed = 0.f;

	UPROPERTY(Transient)
	float CameraInterpDuration = 0.f;
	
	UPROPERTY(Transient)
	FVector CameraInterpStartLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator CameraInterpStartRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FVector CameraInterpolateTargetLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator CameraInterpolateTargetRotation = FRotator::ZeroRotator;
	
	//Handle input
	virtual void Move(const FInputActionValue& Value);

	virtual void Look(const FInputActionValue& Value);

	virtual void UseFirstAttackComponent();

	virtual void UseSecondAttackComponent();

	virtual void Interact(const FInputActionValue& Value);
	
	bool bShouldUseLookInput = true;
	
	bool bShouldUseMoveInput = true;

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

	//Handle components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Ability")
	UAttackComponentBase* FirstAttackComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Ability")
	UAttackComponentBase* SecondAttackComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Misc")
	TObjectPtr<UInteractorComponent> InteractorComponent;
	
	//Handle nametag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Misc")
	TObjectPtr<UWidgetComponent> PlayerNameTagWidgetComponent;

	UPROPERTY(ReplicatedUsing=OnRep_CustomPlayerName, VisibleAnywhere)
	FString CustomPlayerName = TEXT("Player");
	
	UPROPERTY(Replicated, VisibleAnywhere)
	bool bChangedName = false;
	
	//Handle sockets
	UPROPERTY(VisibleAnywhere, Category="Socket Names")
	FName RightHandSocket = TEXT("HandGrip_R");

	UPROPERTY(VisibleAnywhere, Category="Socket Names")
	FName LeftHandSocket = TEXT("HandGrip_L");
	
	//TODO: Remove unused functions 
	UFUNCTION(Server, Reliable)
	void Server_SpawnHitParticles();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnHitParticles();
private:
	//Handle nametag	
	UFUNCTION()
	void OnRep_CustomPlayerName();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetCustomPlayerName(const FString& InPlayerName);

	UFUNCTION(BlueprintCallable)
	void SetUpLocalCustomPlayerName();

	//Handle editor debug
#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, Category="Player Name Debug")
	bool bUsePlayerLoginProfile = false;
	
#endif
};
