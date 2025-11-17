// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "AttributeData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UpgradeSubsystem.generated.h"

using FAttributeData = FAttributeBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnModified, FAttributeData);


UCLASS()
class UPGRADE_API UUpgradeSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	static UUpgradeSubsystem* Get(const UWorld* WorldContextObject);

	UUpgradeSubsystem(){};

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void BindAttribute(UObject* Owner, FName PropertyName, FName RowName, FName Category);

	UFUNCTION(Server, Reliable)
	void Server_Bind(UObject* Owner, FName PropertyName, FName RowName, FName Category);

	//void BindDependentAttribute(UObject* Owner, FName PropertyName, bool OverrideOnModified, UObject* TargetOwner, FName TargetPropertyName);

	void LocalUpgradeByRow(FName RowName) const;
	void LocalDowngradeByRow(FName RowName) const;

	UFUNCTION(Server, Reliable)
	void Server_DowngradeByRow(FName RowName) const;

	UFUNCTION(Server, Reliable)
	void Server_UpgradeByRow(FName RowName) const;

	FAttributeData* GetAttributeData(UObject* Owner, FName PropertyName) const;
//protected:
	FAttributeData* GetByKey(UObject* Owner, FProperty* Property) const;
	const FAttributeData* GetByCategory(FName Category, FName RowName) const;
	TArray<const FAttributeData*> GetByRow(FName RowName) const;
	static uint64 GetKey(UObject* Owner, FProperty* Property);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticast_LoadDataTable();

private:
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Upgrades")
	UDataTable* UpgradeDataTable = nullptr;

	//Listan med alla attribut som ska kunna uppgraderas
	TArray<TUniquePtr<FAttributeData>> RegisteredAttributes;

	//Listan med alla attribut som är beroende av andra attribut
	TArray<TUniquePtr<FDependentAttribute>> RegisteredDependentAttributes;

	//Ifall det ska finnas flera attribut som ska uppgraderas samtidigt (samma namn)
	TMap<FName, TArray<FAttributeData*>> AttributesByRow;

	//För uppgradering av enstaka attribut
	TMap<uint64, FAttributeData*> AttributesByKey;

	//För uppgradering av enstaka attribut
	TMap<FName, TArray<FAttributeData*>> AttributesByCategory;

protected:
	void ClearAttributes(const FString& String);
};
