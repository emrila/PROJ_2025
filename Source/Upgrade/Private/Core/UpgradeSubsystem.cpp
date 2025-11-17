// Furkan approves of this


#include "Core/UpgradeSubsystem.h"

#include "Core/UpgradeDisplayData.h"
#include "Dev/UpgradeLog.h"

#include "Net/UnrealNetwork.h"

namespace UpgradeCheck
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
		return FString::Printf(TEXT("%s_%s"), *Object->GetName(), *Object->GetName());
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

UUpgradeSubsystem* UUpgradeSubsystem::Get(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	/*const UGameInstance* GameInstance = World->GetGameInstance();
	if (GameInstance)
	{
		return GameInstance->GetSubsystem<UUpgradeSubsystem>();
	}*/
	if (const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
	{
		return LocalPlayer->GetSubsystem<UUpgradeSubsystem>();
	}

	return nullptr;
}

void UUpgradeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NetMulticast_LoadDataTable();
}

void UUpgradeSubsystem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUpgradeSubsystem, UpgradeDataTable);
}

void UUpgradeSubsystem::BindAttribute(UObject* Owner, const FName PropertyName, const FName RowName, const FName Category)
{
	//UPGRADE_HI_FROM(__FUNCTION__);
	if (!Owner)
	{
		UPGRADE_ERROR(TEXT("%hs: Owner is null!"), __FUNCTION__);
		return;
	}
	FProperty* Prop = UpgradeCheck::GetProperty(Owner, PropertyName);
	if (!FAttributeData::IsValidProperty(Prop))
	{
		UPGRADE_ERROR(TEXT("%hs: Property %s is invalid on owner %s!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeCheck::GetClassNameKey(Owner));		
		return;
	}
	/*if (const FAttributeData* ExistingAttribute = GetByCategory(Category, RowName))
	{
		BindDependentAttribute(Owner, PropertyName, false, ExistingAttribute->Owner.Get(), ExistingAttribute->Property->GetFName());
		return;
	}*/

	const uint64 Key = GetKey(Owner, Prop);
	if (AttributesByKey.Contains(Key))
	{
		UPGRADE_WARNING(TEXT("%hs: Attribute for property %s on owner %s is already bound!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeCheck::GetClassNameKey(Owner));
	}
	const FAttributeUpgradeData* UpgradeData = UpgradeDataTable->FindRow<FAttributeUpgradeData>(RowName, __FUNCTION__);
	if (!UpgradeData)
	{
		UPGRADE_ERROR(TEXT("%hs: No upgrade data found for row %s!"), __FUNCTION__, *RowName.ToString());
		return;
	}

	TUniquePtr<FAttributeData> NewAttribute = UpgradeCheck::CreateAttribute(Owner, Prop, RowName);
	if (!NewAttribute)
	{
		UPGRADE_ERROR(TEXT("%hs: Failed to create attribute for property %s on owner %s!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeCheck::GetClassNameKey(Owner));
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
			UPGRADE_DISPLAY(TEXT("%hs: Attribute %s has reached max upgrade level %d."), __FUNCTION__, *UpgradeCheck::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
			return;
		}
		NewAttributeRaw->Modify(FModiferData{UpgradeData->Multiplier});
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
			UPGRADE_DISPLAY(TEXT("%hs: Attribute %s has reached min downgrade level %d."), __FUNCTION__, *UpgradeCheck::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
			return;
		}
		NewAttributeRaw->Modify(FModiferData{UpgradeData->Multiplier, true});
	});

	AttributesByRow.FindOrAdd(RowName).Add(NewAttributeRaw);
	AttributesByKey.Add(Key, NewAttributeRaw);
	AttributesByCategory.FindOrAdd(Category).Add(NewAttributeRaw);
	RegisteredAttributes.Add(MoveTemp(NewAttribute));

	UPGRADE_DISPLAY(TEXT("%hs: Bound attribute %s with row %s under category %s. Key %d"), __FUNCTION__,*UpgradeCheck::GetClassNameKey(Owner), *RowName.ToString(), *Category.ToString(), Key);

}

void UUpgradeSubsystem::Server_Bind_Implementation(UObject* Owner, FName PropertyName, FName RowName, FName Category)
{
	BindAttribute(Owner, PropertyName, RowName, Category);
}

void UUpgradeSubsystem::LocalDowngradeByRow(const FName RowName) const
{
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnRemoveModifier.Broadcast();
		if (TargetAttribute->OnAttributeModified.IsBound())
		{
			TargetAttribute->OnAttributeModified.Broadcast();
		}
	}
}

void UUpgradeSubsystem::LocalUpgradeByRow(const FName RowName) const
{
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnAddModifier.Broadcast();
		if (TargetAttribute->OnAttributeModified.IsBound())
		{
			TargetAttribute->OnAttributeModified.Broadcast();
		}
	}
}

/*
void UUpgradeSubsystem::BindDependentAttribute(UObject* Owner, const FName PropertyName, bool OverrideOnModified, UObject* TargetOwner,	const FName TargetPropertyName)
{
	if (!Owner)
	{
		return;
	}
	FProperty* Prop = UpgradeCheck::GetProperty(Owner, PropertyName);
	if (!UpgradeCheck::IsValidProperty(Prop))
	{
		return;
	}

	FAttributeData* TargetAttribute = GetByKey(TargetOwner, UpgradeCheck::GetProperty(TargetOwner, TargetPropertyName));
	if (!TargetAttribute)
	{
		return;
	}
	const FAttributeUpgradeData* UpgradeData = UpgradeDataTable->FindRow<FAttributeUpgradeData>(TargetAttribute->RowName, __FUNCTION__);
	if (!UpgradeData)
	{
		return;
	}
	TUniquePtr<FDependentAttribute> NewDependentAttribute = MakeUnique<FDependentAttribute>(Owner, Prop, OverrideOnModified);
	FDependentAttribute* DependentAttributeRaw = NewDependentAttribute.Get();
	TargetAttribute->OnAttributeModified.AddLambda([TargetAttribute, UpgradeData, DependentAttributeRaw]
	{
		if (!DependentAttributeRaw->Owner.IsValid() || !TargetAttribute->Owner.IsValid())
		{
			return;
		}

		UObject* DepOwner = DependentAttributeRaw->Owner.Get();
		const UObject* TaOwner = TargetAttribute->Owner.Get();

		if (!IsValid(DepOwner) || !IsValid(TaOwner))
		{
			return;
		}

		// ---------FLOAT SPECIFIC
		float Current = 0.f;
		UpgradeCheck::GetFloatValue(Current, DependentAttributeRaw->Property, DepOwner);
		if (DependentAttributeRaw->bOverrideOnModified)
		{
			// ---------FLOAT SPECIFIC
			float TargetsCurrent =	0.f;
			UpgradeCheck::GetFloatValue(TargetsCurrent, TargetAttribute->Property, TaOwner);
			Current = TargetsCurrent;
		}
		else
		{
			// ---------FLOAT SPECIFIC
			Current += TargetAttribute->InitialValue * UpgradeData->Multiplier;
		}
		// ---------FLOAT SPECIFIC
		UpgradeCheck::SetFloatValue(Current, DependentAttributeRaw->Property, DepOwner);
		//----------------------------------
	});

	RegisteredDependentAttributes.Add(MoveTemp(NewDependentAttribute));
}
*/

void UUpgradeSubsystem::Server_DowngradeByRow_Implementation(const FName RowName) const
{
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnRemoveModifier.Broadcast();
	}
}

void UUpgradeSubsystem::Server_UpgradeByRow_Implementation(const FName RowName) const
{
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnAddModifier.Broadcast();
	}
}

FAttributeData* UUpgradeSubsystem::GetAttributeData(UObject* Owner, FName PropertyName) const
{
	FProperty* Prop = UpgradeCheck::GetProperty(Owner, PropertyName);
	if (!Prop)
	{
		return nullptr;
	}
	return GetByKey(Owner, Prop);
}

FAttributeData* UUpgradeSubsystem::GetByKey(UObject* Owner, FProperty* Property) const
{
	return AttributesByKey.FindRef(GetKey(Owner, Property));
}

const FAttributeData* UUpgradeSubsystem::GetByCategory(FName Category, FName RowName) const
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

TArray<const FAttributeData*> UUpgradeSubsystem::GetByRow(FName RowName) const
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

uint64 UUpgradeSubsystem::GetKey(UObject* Owner, FProperty* Property)
{
	return reinterpret_cast<uint64>(Owner) << 32 ^ reinterpret_cast<uint64>(Property);
}

void UUpgradeSubsystem::NetMulticast_LoadDataTable_Implementation()
{
	if (UpgradeDataTable)
	{
		UPGRADE_WARNING(TEXT("%hs: UpgradeDataTable already loaded."), __FUNCTION__);
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

bool UUpgradeSubsystem::NetMulticast_LoadDataTable_Validate()
{
	return true;
}

void UUpgradeSubsystem::ClearAttributes([[maybe_unused]] const FString& String)
{
	UPGRADE_HI_FROM(__FUNCTION__);
	RegisteredAttributes.Empty();
	AttributesByRow.Empty();
	AttributesByKey.Empty();
	AttributesByCategory.Empty();
	RegisteredDependentAttributes.Empty();
}
