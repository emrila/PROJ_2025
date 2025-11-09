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
		return Property && Property->IsA<FFloatProperty>();
	}

	FFloatProperty* GetFloatProperty(float& OutValue, FProperty* Property, const UObject* Owner)
	{
		if (!IsValidProperty(Property))
		{
			return nullptr;
		}

		FFloatProperty* FloatProp = CastFieldChecked<FFloatProperty>(Property);
		OutValue = FloatProp->GetPropertyValue_InContainer(Owner);
		return FloatProp;
	}
	void GetFloatValue(float& OutValue, FProperty* Property, const UObject* Owner)
	{
		const FFloatProperty* FloatProp = CastFieldChecked<FFloatProperty>(Property);
		OutValue = FloatProp->GetPropertyValue_InContainer(Owner);
	}
	void SetFloatValue(float InValue, FProperty* Property, UObject* Owner)
	{
		if (!IsValidProperty(Property))
		{
			return;
		}
		const FFloatProperty* FloatProp = CastFieldChecked<FFloatProperty>(Property);
		FloatProp->SetPropertyValue_InContainer(Owner, InValue);
}

	FString GetClassNameKey(const UObject* Object)
	{
		return FString::Printf(TEXT("%s_%s"), *Object->GetClass()->GetName(), *Object->GetName());
	}
}

#define TABLE_PATH TEXT("/Game/Developer/Emma/DT_UpgradeData.DT_UpgradeData")

UUpgradeSubsystem* UUpgradeSubsystem::Get(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	if (const UGameInstance* GameInstance = World->GetGameInstance())
	{
		return GameInstance->GetSubsystem<UUpgradeSubsystem>();
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

void UUpgradeSubsystem::BindAttribute(UObject* Owner, const FName PropertyName, FName RowName, FName Category, bool bFindOnReload)
{
	UPGRADE_HI_FROM(__FUNCTION__);

	if (!Owner)
	{
		UPGRADE_ERROR(TEXT("%hs: Owner is null!"), __FUNCTION__);
		return;
	}

	FProperty* Prop = UpgradeCheck::GetProperty(Owner, PropertyName);
	if (!UpgradeCheck::IsValidProperty(Prop))
	{
		UPGRADE_ERROR(TEXT("%hs: Property %s is invalid on owner %s!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeCheck::GetClassNameKey(Owner));
		return;
	}

	if (const FAttributeData* ExistingAttribute = GetByCategory(Category, RowName))
	{
		BindDependentAttribute(Owner, PropertyName, false, ExistingAttribute->Owner.Get(),ExistingAttribute->Property->GetFName());
		return;
	}

	const uint64 Key = GetKey(Owner, Prop);
	const FAttributeUpgradeData* UpgradeData = UpgradeDataTable->FindRow<FAttributeUpgradeData>(RowName, __FUNCTION__);
	if (!UpgradeData)
	{
		UPGRADE_ERROR(TEXT("%hs: No upgrade data found for row %s!"), __FUNCTION__, *RowName.ToString());
		return;
	}

	float InitialValue = 0.f;
	UpgradeCheck::GetFloatValue(InitialValue, Prop, Owner);
	TUniquePtr<FAttributeData> NewAttribute = MakeUnique<FAttributeData>(Owner, Prop, RowName, InitialValue);
	FAttributeData* NewAttributeRaw = NewAttribute.Get();
	NewAttributeRaw->OnAttributeModified.AddLambda([NewAttributeRaw, UpgradeData]
	{
		if (!NewAttributeRaw->Owner.IsValid())
		{
			UPGRADE_ERROR(TEXT("%hs: Owner is no longer valid!"), __FUNCTION__);
			return;
		}

		if (UpgradeData->MaxNumberOfUpgrades <= NewAttributeRaw->CurrentUpgradeLevel && UpgradeData->MaxNumberOfUpgrades != -1)
		{
			UPGRADE_DISPLAY(TEXT("%hs: Attribute %s has reached max upgrade level %d."), __FUNCTION__,
				*UpgradeCheck::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
			return;
		}
		NewAttributeRaw->CurrentUpgradeLevel++;

		float Current = 0.f;
		UpgradeCheck::GetFloatValue(Current, NewAttributeRaw->Property, NewAttributeRaw->Owner.Get());
		Current += NewAttributeRaw->InitialValue * UpgradeData->Multiplier;
		UpgradeCheck::SetFloatValue(Current, NewAttributeRaw->Property, NewAttributeRaw->Owner.Get());

		UPGRADE_DISPLAY(TEXT("%hs: Upgraded attribute %s to level %d. New value: %f"), __FUNCTION__,
			*UpgradeCheck::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel, Current);
	});

	// Lägg till i alla listor/loop-ups
	AttributesByRow.FindOrAdd(RowName).Add(NewAttributeRaw);
	AttributesByKey.Add(Key, NewAttributeRaw);
	AttributesByCategory.FindOrAdd(Category).Add(NewAttributeRaw);
	RegisteredAttributes.Add(MoveTemp(NewAttribute));

	UPGRADE_DISPLAY(TEXT("%hs: Bound attribute %s with row %s under category %s."), __FUNCTION__,
		*UpgradeCheck::GetClassNameKey(Owner), *RowName.ToString(), *Category.ToString());

}

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
		float Current = 0.f;
		UpgradeCheck::GetFloatValue(Current, DependentAttributeRaw->Property, DepOwner);
		if (DependentAttributeRaw->bOverrideOnModified)
		{
			float TargetsCurrent =	0.f;
			UpgradeCheck::GetFloatValue(TargetsCurrent, TargetAttribute->Property, TaOwner);
			Current = TargetsCurrent;
		}
		else
		{
			Current += TargetAttribute->InitialValue * UpgradeData->Multiplier;
		}

		UpgradeCheck::SetFloatValue(Current, DependentAttributeRaw->Property, DepOwner);
	});

	RegisteredDependentAttributes.Add(MoveTemp(NewDependentAttribute));
}

void UUpgradeSubsystem::UpgradeByRow(FName RowName) const
{
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		TargetAttribute->OnAttributeModified.Broadcast();
	}
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
