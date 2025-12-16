#pragma once

#include "NiagaraFunctionLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacterBase.generated.h"

class UInventory;
struct FInputActionInstance;
class UUpgradeComponent;
class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class UInteractorComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UAttackComponentBase;

DECLARE_LOG_CATEGORY_EXTERN(PlayerBaseLog, Log, All);

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDash);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDied, bool, bNewIsAlive);

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIFrameChanged, bool, bIFrameActive);

UDELEGATE(BlueprintCallable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIFrameChangedVisuals, bool, bIFrameActive);

UCLASS()
class PROJ_2025_API APlayerCharacterBase : public ACharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacterBase();
	
	//Handle override parent functions
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	//Handle components
	UFUNCTION(BlueprintCallable)
	UAttackComponentBase* GetBasicAttackComponent() const;

	UFUNCTION(BlueprintCallable)
	UAttackComponentBase* GetSpecialAttackComponent() const;

	// Inventory pointer
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	UInventory* Inventory;
	
	//Handle sockets
	FVector GetRightHandSocketLocation() const;

	FVector GetLeftHandSocketLocation() const;
	
	AActor* GetRightHandAttachedActor() const;
	
	//Handle camera
	virtual void HandleCameraDetachment();
	
	virtual void HandleCameraReattachment();
	
	//Handle Input
	virtual void SetInputActive(const bool bNewInputActive);
	
	virtual void SetShouldUseSprintInput(const bool bNewShouldUseInput);
	
	virtual void EndSprint();
	
	virtual bool IsInputActive() const { return bIsInputActive; }
	
	virtual bool GetShouldUseSprintInput() const { return bShouldUseSprintInput; }
	
	//Handle Damage
	virtual void StartIFrame();
	
	virtual void StartIFrameVisuals();
	
	virtual void ResetIFrame();
	
	virtual void ResetIFrameVisuals();

	virtual bool IsAlive() const { return bIsAlive; }

	virtual void SetIsAlive(const bool NewIsAlive);

	FOnPlayerDied OnPlayerDied;
	
	UPROPERTY(BlueprintAssignable)
	FOnIFrameChanged OnIFrameChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnIFrameChangedVisuals OnIFrameChangedVisuals;
	
	UPROPERTY(BlueprintAssignable)
	FOnDash OnDash;
	
	/*UFUNCTION(Client, Reliable)
	virtual void Client_StartCameraInterpolation(
		const FVector& TargetLocation, const FRotator& TargetRotation, const float LerpDuration);*/
	
	UFUNCTION(Client, Reliable)
	virtual void Client_StartCameraInterpolation(
		const FVector& TargetLocation, const float LerpDuration);

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//TODO: This may be moved to the belonging attack component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Particles")
	UNiagaraSystem* ImpactParticles;

	UFUNCTION()
	void StartSuddenDeath();

	UFUNCTION()
	void EndSuddenDeath();

	UFUNCTION(BlueprintImplementableEvent)
	void HitFeedback();

	UFUNCTION(Server, Reliable)
	void Server_HitFeedback();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HitFeedback();
	
	UFUNCTION(Client, Reliable)
	void Client_ShowDamageVignette();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> DamageVignetteWidget;

protected:
	//Handle override parent functions
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,AActor* DamageCauser) override;
	
	//Handle take damage
	UPROPERTY(Replicated)
	bool IFrame = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Damage")
	float DefenceStat = 0.f;
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
	virtual void InterpolateCamera(FVector& TargetLocation, const float LerpDuration);
	
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
	
	virtual void Jump() override;
	
	FTimerHandle JumpTimerHandle;
	
	virtual void OnSprintBegin(const FInputActionInstance& ActionInstance);
	
	virtual void OnSprintEnd(const FInputActionInstance& ActionInstance);

	virtual void Interact(const FInputActionValue& Value);
	
	virtual void SetupAttackComponentInput(UEnhancedInputComponent* EnhancedInputComponent);
	
	bool bShouldUseLookInput = true;
	
	bool bShouldUseMoveInput = true;
	
	bool bIsInputActive = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Movement")
	bool bShouldUseSprintInput = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Movement")
	float SprintSpeedMultiplier = 1.5f;
	
	bool bShouldSprint = false;
	
	UFUNCTION(Server, Reliable)
	void Server_SetSprint(const bool bNewSprintState);
	
	float CurrentMaxWalkSpeed = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, meta = (AllowPrivateAccess = true))
	bool bIsAlive = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* MouseLookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* JumpAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Movement")
	UInputAction* SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Ability")
	UInputAction* BasicAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Ability")
	UInputAction* SpecialAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input|Misc")
	UInputAction* InteractAction;

	//Handle components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Components|Ability")
	UAttackComponentBase* BasicAttackComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Components|Ability")
	UAttackComponentBase* SpecialAttackComponent;
	
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Ability")
	TSubclassOf<UAttackComponentBase> BasicAttackComponentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Ability")
	TSubclassOf<UAttackComponentBase> SpecialAttackComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Misc")
	TObjectPtr<UInteractorComponent> InteractorComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Misc")
	TObjectPtr<UUpgradeComponent> UpgradeComponent;
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category="Components|Misc")
	void SetupBindAttributes();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	bool bIsAttacking = false;
	
	//Handle nametag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components|Misc")
	TObjectPtr<UWidgetComponent> PlayerNameTagWidgetComponent;

	UPROPERTY(ReplicatedUsing=OnRep_CustomPlayerName, VisibleAnywhere)
	FString CustomPlayerName = TEXT("Player");
	
	UPROPERTY(Replicated, VisibleAnywhere)
	bool bChangedName = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = true))
	FText ClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = true))
	FText ClassDescription;
	
	//Handle sockets
	UPROPERTY(VisibleAnywhere, Category="Socket Names")
	FName RightHandSocket = TEXT("R_HandSocket");

	UPROPERTY(VisibleAnywhere, Category="Socket Names")
	FName LeftHandSocket = TEXT("L_HandSocket");
	
	//TODO: Remove unused functions 
	UFUNCTION(Server, Reliable)
	void Server_SpawnEffect(const FVector& EffectSpawnLocation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEffect(const FVector& EffectSpawnLocation);

private:
	//Handle nametag	
	UFUNCTION()
	void OnRep_CustomPlayerName();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetCustomPlayerName(const FString& InPlayerName);

	UFUNCTION(BlueprintCallable)
	void SetUpLocalCustomPlayerName();

	UPROPERTY(Replicated)
	bool SuddenDeath;

	

	//Handle editor debug
#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere, Category="Player Name Debug")
	bool bUsePlayerLoginProfile = false;
	
#endif
};
