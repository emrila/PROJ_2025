// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UpgradeSubsystem.h"
#include "Components/ActorComponent.h"
#include "UpgradeComponent.generated.h"

using FAttributeData = FAttributeBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UPGRADE_API UUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUpgradeComponent();

	virtual void BeginPlay() override;	

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable)
	void BindAttribute(UObject* Owner, FName PropertyName, FName RowName, FName Category);
	UFUNCTION(Server, Reliable)
	void UpgradeByRow(FName RowName) const;	
	void DowngradeByRow(FName RowName) const;

protected:
	FAttributeData* GetByKey(UObject* Owner, FProperty* Property) const;
	const FAttributeData* GetByCategory(FName Category, FName RowName) const;
	TArray<const FAttributeData*> GetByRow(FName RowName) const;
	static uint64 GetKey(UObject* Owner, FProperty* Property);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticast_LoadDataTable();

private:
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Upgrades")
	UDataTable* UpgradeDataTable = nullptr;

	TArray<TUniquePtr<FAttributeData>> RegisteredAttributes;

	TArray<TUniquePtr<FDependentAttribute>> RegisteredDependentAttributes;

	TMap<FName, TArray<FAttributeData*>> AttributesByRow;

	TMap<uint64, FAttributeData*> AttributesByKey;

	TMap<FName, TArray<FAttributeData*>> AttributesByCategory;

protected:
	void ClearAttributes(const FString& String);

};
