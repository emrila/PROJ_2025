// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "../Data/AttributeData.h"
#include "../Data/UpgradeDisplayData.h"
#include "../Data/UpgradeEvents.h"
#include "Components/ActorComponent.h"
#include "Data/TeamModifierData.h"
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
	void Server_BindAttribute(UObject* Owner, FName PropertyName, FName RowName);

	UFUNCTION(Server, Reliable)
	void Server_UpgradeByRow(FName RowName);
	void DowngradeByRow(FName RowName) const;

	UFUNCTION()
	void OnUpgradeReceived(FInstancedStruct InstancedStruct);

	UFUNCTION(Server, Reliable)
	void Server_OnUpgradeReceived(FInstancedStruct InstancedStruct);


	UFUNCTION(BlueprintCallable)
	TArray<FUpgradeDisplayData> GetRandomUpgrades(const int32 NumberOfUpgrades);
	
	UFUNCTION(BlueprintCallable)
	FTeamModifierData GetTeamModifier(FName RowName);

	FAttributeData* GetByKey(UObject* Owner, FProperty* Property) const;

	UFUNCTION(BlueprintPure)
	TArray<FUpgradeDisplayData> GetPlayerUpgrades();

protected:
	const FAttributeData* GetByCategory(FName Category, FName RowName) const;
	TArray<const FAttributeData*> GetByRow(FName RowName) const;
	
	UFUNCTION(Server, Reliable)
	void Server_LoadDataTable();	

private:
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Upgrades")
	UDataTable* UpgradeDataTable = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Upgrades")
	UDataTable* TeamModifierDataTable = nullptr;

	TArray<TUniquePtr<FAttributeData>> RegisteredAttributes;

	TMap<FName, TArray<FAttributeData*>> AttributesByRow;

	TMap<uint64, FAttributeData*> AttributesByKey;

	TMap<FName, TArray<FAttributeData*>> AttributesByCategory;

protected:
	void ClearAttributes(const FString& String);
	
public:
	FOnUpgradeEvent OnUpgraded;
};
