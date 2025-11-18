// Furkan approves of this


#include "Core/UpgradeComponent.h"

#include "Core/UpgradeDisplayData.h"
#include "Dev/UpgradeLog.h"
#include "Net/UnrealNetwork.h"


struct FAttributeUpgradeData;

namespace UpgradeUtils
{
	FProperty* GetProperty(const UObject* Owner, const FName PropertyName)
	{
		if (!Owner)
		{
			return nullptr;
		}

		return Owner->GetClass()->FindPropertyByName(PropertyName);
	}

	bool IsValidProperty(const FProperty* Property)
	{
		return Property && (
			Property->IsA<FFloatProperty>() ||
			Property->IsA<FIntProperty>()
		);
	}

	FString GetClassNameKey(const UObject* Object)
	{
		return FString::Printf(TEXT("%s_%s"), *Object->GetClass()->GetName(), *Object->GetName());
	}

	static TUniquePtr<FAttributeBase> CreateAttribute(UObject* InOwner, FProperty* InProperty, const FName InRowName)
	{
		if (!InProperty)
		{
			return nullptr;
		}

		if (InProperty->IsA<FFloatProperty>())
		{
			return MakeUnique<FAttributeFloat>(InOwner, InProperty, InRowName);
		}
		if (InProperty->IsA<FIntProperty>())
		{
			return MakeUnique<FAttributeInt32>(InOwner, InProperty, InRowName);
		}

		return nullptr;
	}
}

#define TABLE_PATH TEXT("/Game/Developer/Emma/DT_UpgradeData.DT_UpgradeData")

UUpgradeComponent::UUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
	NetMulticast_LoadDataTable();

	bHasAppliedUpgrade = false;
}

void UUpgradeComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUpgradeComponent, UpgradeDataTable);
	DOREPLIFETIME(UUpgradeComponent, bHasAppliedUpgrade);
}

void UUpgradeComponent::BindAttribute_Implementation(UObject* Owner, FName PropertyName, FName RowName, FName Category)
{
	UPGRADE_HI_FROM(__FUNCTION__);

	if (!Owner)
	{
		UPGRADE_ERROR(TEXT("%hs: Owner is null!"), __FUNCTION__);
		return;
	}
	FProperty* Prop = UpgradeUtils::GetProperty(Owner, PropertyName);
	if (!FAttributeData::IsValidProperty(Prop))
	{
		UPGRADE_ERROR(TEXT("%hs: Property %s is invalid on owner %s!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeUtils::GetClassNameKey(Owner));		
		return;
	}
	/*if (const FAttributeData* ExistingAttribute = GetByCategory(Category, RowName))
	{
		BindDependentAttribute(Owner, PropertyName, false, ExistingAttribute->Owner.Get(),ExistingAttribute->Property->GetFName());
		return;
	}*/

	const uint64 Key = GetKey(Owner, Prop);
	const FAttributeUpgradeData* UpgradeData = UpgradeDataTable->FindRow<FAttributeUpgradeData>(RowName, __FUNCTION__);
	if (!UpgradeData)
	{
		UPGRADE_ERROR(TEXT("%hs: No upgrade data found for row %s!"), __FUNCTION__, *RowName.ToString());
		return;
	}

	TUniquePtr<FAttributeData> NewAttribute = UpgradeUtils::CreateAttribute(Owner, Prop, RowName);//MakeUnique<FAttributeData>(Owner, Prop, RowName, InitialValue);
	if (!NewAttribute)
	{
		UPGRADE_ERROR(TEXT("%hs: Failed to create attribute for property %s on owner %s!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeUtils::GetClassNameKey(Owner));
		return;
	}

	FAttributeData* NewAttributeRaw = NewAttribute.Get();
	
	NewAttributeRaw->OnAddModifier.AddLambda([NewAttributeRaw, UpgradeData]
	{
		if (!NewAttributeRaw->Owner.IsValid())
		{
			UPGRADE_ERROR(TEXT("%hs: Owner is no longer valid!"), __FUNCTION__);
			return;
		}
		if (!UpgradeData->CanUpgrade(NewAttributeRaw->CurrentUpgradeLevel))
		{
			UPGRADE_DISPLAY(TEXT("%hs: Attribute %s has reached max upgrade level %d."), __FUNCTION__, *UpgradeUtils::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
			return;
		}
	
		NewAttributeRaw->Modify(FModiferData{UpgradeData->Multiplier});
		UPGRADE_DISPLAY( TEXT("%hs: Upgraded attribute %s to level %d."), __FUNCTION__, *UpgradeUtils::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
	});
	NewAttributeRaw->OnRemoveModifier.AddLambda([NewAttributeRaw, UpgradeData]
	{
		if (!NewAttributeRaw->Owner.IsValid())
		{
			UPGRADE_ERROR(TEXT("%hs: Owner is no longer valid!"), __FUNCTION__);
			return;
		}
		if (!UpgradeData->CanDowngrade(NewAttributeRaw->CurrentUpgradeLevel))
		{
			UPGRADE_DISPLAY(TEXT("%hs: Attribute %s has reached min downgrade level %d."), __FUNCTION__, *UpgradeUtils::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
			return;
		}
		NewAttributeRaw->Modify(FModiferData{UpgradeData->Multiplier, true});
	
	});

	// Lägg till i alla listor/loop-ups
	AttributesByRow.FindOrAdd(RowName).Add(NewAttributeRaw);
	AttributesByKey.Add(Key, NewAttributeRaw);
	AttributesByCategory.FindOrAdd(Category).Add(NewAttributeRaw);
	RegisteredAttributes.Add(MoveTemp(NewAttribute));

	UPGRADE_DISPLAY(TEXT("%hs: Bound attribute %s with row %s under category %s."), __FUNCTION__,*UpgradeUtils::GetClassNameKey(Owner), *RowName.ToString(), *Category.ToString());
}

void UUpgradeComponent::UpgradeByRow_Implementation(FName RowName)
{
	if (bHasAppliedUpgrade)
	{
		return;
	}
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnAddModifier.Broadcast();
		UPGRADE_DISPLAY( TEXT("%hs: Upgraded attribute %s with row %s."), __FUNCTION__, *UpgradeUtils::GetClassNameKey(TargetAttribute->Owner.Get()), *RowName.ToString());
	}
	
	bHasAppliedUpgrade = true;
}

void UUpgradeComponent::DowngradeByRow(FName RowName) const
{
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnRemoveModifier.Broadcast();
	}
}

void UUpgradeComponent::OnUpgradeReceived(FInstancedStruct InstancedStruct)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	const FString StructCPPName = InstancedStruct.GetScriptStruct()->GetStructCPPName();
	UPGRADE_DISPLAY(TEXT("%hs: Received upgrade struct of type %s."), __FUNCTION__, *StructCPPName);
	FUpgradeDisplayData* UpgradeDataPtr = InstancedStruct.GetMutablePtr<FUpgradeDisplayData>();
	if (!UpgradeDataPtr)
	{
		UPGRADE_ERROR(TEXT("%hs: Failed to get FUpgradeDisplayData from InstancedStruct!"), __FUNCTION__);
		return;
	}
	const FUpgradeDisplayData& UpgradeData = *UpgradeDataPtr;
	
	UpgradeByRow(UpgradeData.RowName);
	
}

FAttributeData* UUpgradeComponent::GetByKey(UObject* Owner, FProperty* Property) const
{
	return AttributesByKey.FindRef(GetKey(Owner, Property));

}

const FAttributeData* UUpgradeComponent::GetByCategory(FName Category, FName RowName) const
{
	const TArray<FAttributeData*>* Attributes = AttributesByCategory.Find(Category);
	if (!Attributes)
	{
		return nullptr;
	}

	FAttributeData* const* TargetAttribute = Attributes->FindByPredicate([RowName](const FAttributeData* Attri)
	{
		return Attri->RowName == RowName;
	});

	return TargetAttribute ? *TargetAttribute : nullptr;
}

TArray<const FAttributeData*> UUpgradeComponent::GetByRow(FName RowName) const
{
	TArray<const FAttributeData*> Out;
	if (const TArray<FAttributeData*>* Arr = AttributesByRow.Find(RowName))
	{
		for (const FAttributeData* Mod : *Arr)
		{
			Out.Add(Mod);
		}
	}
	return Out;
}

uint64 UUpgradeComponent::GetKey(UObject* Owner, FProperty* Property)
{
	return reinterpret_cast<uint64>(Owner) << 32 ^ reinterpret_cast<uint64>(Property);
}

void UUpgradeComponent::NetMulticast_LoadDataTable_Implementation()
{
	if (UpgradeDataTable)
	{
		UPGRADE_DISPLAY(TEXT("%hs: UpgradeDataTable already loaded."), __FUNCTION__);
		return;
	}
	UpgradeDataTable = LoadObject<UDataTable>(nullptr, TABLE_PATH);
	if (!UpgradeDataTable)
	{
		UPGRADE_ERROR(TEXT("%hs: Failed to load UpgradeDataTable at path: %s"), __FUNCTION__, TABLE_PATH);
	}
	else
	{
		UPGRADE_DISPLAY(TEXT("%hs: Successfully loaded UpgradeDataTable."), __FUNCTION__);
	}
}

bool UUpgradeComponent::NetMulticast_LoadDataTable_Validate()
{
	return true;
}

void UUpgradeComponent::ClearAttributes(const FString& String)
{
	UPGRADE_HI_FROM(__FUNCTION__);
	RegisteredAttributes.Empty();
	AttributesByRow.Empty();
	AttributesByKey.Empty();
	AttributesByCategory.Empty();
	RegisteredDependentAttributes.Empty();
}

