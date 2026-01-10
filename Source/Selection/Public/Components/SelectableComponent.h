// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "SelectionComponent.h"
#include "SelectableComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SELECTION_API USelectableComponent : public USelectionComponent
{
	GENERATED_BODY()

public:
	USelectableComponent() = default;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void OnRegisterSelectable_Implementation(FInstancedStruct RegistrationData) override;
	virtual void OnUnregisterSelectable_Implementation(FInstancedStruct UnregistrationData) override;
	virtual bool CanRegister_Implementation() const override;

	virtual void OnRequestSelection_Implementation(FInstancedStruct RequestData) override;
private:
	AActor* GetTargetOwner() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Selectable")
	bool bAutoRegister = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Selectable")
	bool bUseOwnersOwnerAsTarget = true;
};
