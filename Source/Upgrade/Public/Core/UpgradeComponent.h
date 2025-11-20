// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "AttributeData.h"
#include "UpgradeDisplayData.h"
#include "UpgradeEvents.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "UpgradeComponent.generated.h"

using FAttributeData = FAttributeBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UPGRADE_API UUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUpgradeComponent();

	virtual void BeginPlay() override;	

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable)
	void BindAttribute(UObject* Owner, FName PropertyName, FName RowName, FName Category);
	
	UFUNCTION(Server, Reliable)
	void UpgradeByRow(FName RowName);	
	void DowngradeByRow(FName RowName) const;

	UFUNCTION()
	void OnUpgradeReceived(FInstancedStruct InstancedStruct);
	
	UFUNCTION(BlueprintCallable)
	TArray<FUpgradeDisplayData> GetRandomUpgrades(const int32 NumberOfUpgrades);
	
protected:
	FAttributeData* GetByKey(UObject* Owner, FProperty* Property) const;
	const FAttributeData* GetByCategory(FName Category, FName RowName) const;
	TArray<const FAttributeData*> GetByRow(FName RowName) const;
	
	UFUNCTION(Server, Reliable)
	void Server_LoadDataTable();

private:
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Upgrades")
	UDataTable* UpgradeDataTable = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Upgrades")
	bool bHasAppliedUpgrade = false;
	
	TArray<TUniquePtr<FAttributeData>> RegisteredAttributes;

	TArray<TUniquePtr<FDependentAttribute>> RegisteredDependentAttributes;

	TMap<FName, TArray<FAttributeData*>> AttributesByRow;

	TMap<uint64, FAttributeData*> AttributesByKey;

	TMap<FName, TArray<FAttributeData*>> AttributesByCategory;

protected:
	void ClearAttributes(const FString& String);
	
public:
	FOnUpgradeEvent OnUpgraded;
};
