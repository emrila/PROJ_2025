// Furkan approves of this


#include "Components/UpgradeComponent.h"
#include "Data/UpgradeDisplayData.h"
#include "Util/UpgradeLog.h"
#include "Flags/UpgradeFlags.h"
#include "Net/UnrealNetwork.h"
#include "Util/UpgradeUtil.h"
#include "Utility/GameplayUtilFunctionLibrary.h"

struct FAttributeUpgradeData;

#define UPGRADE_TABLE_PATH TEXT("/Game/Developer/Emma/DT_UpgradeData.DT_UpgradeData")
#define MODIFIER_TABLE_PATH TEXT("/Game/Developer/Emma/DT_TeamModifierData.DT_TeamModifierData")

UUpgradeComponent::UUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner()->HasAuthority())
	{
		Server_LoadDataTable();
	}
}

void UUpgradeComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUpgradeComponent, UpgradeDataTable);
	DOREPLIFETIME(UUpgradeComponent, TeamModifierDataTable);
}

void UUpgradeComponent::Server_BindAttribute_Implementation(UObject* Owner, const FName PropertyName, const FName RowName)
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

	const uint64 Key = UpgradeUtils::GetKey(Owner, Prop);
	
	if (AttributesByKey.Contains(Key))
	{
		UPGRADE_WARNING(TEXT("%hs: Attribute for property %s on owner %s is already bound!"), __FUNCTION__, *PropertyName.ToString(), *UpgradeUtils::GetClassNameKey(Owner));
		return;
	}
	const FAttributeUpgradeData* UpgradeData = UpgradeDataTable->FindRow<FAttributeUpgradeData>(RowName, __FUNCTION__);
	if (!UpgradeData)
	{
		UPGRADE_ERROR(TEXT("%hs: No upgrade data found for row %s!"), __FUNCTION__, *RowName.ToString());
		return;
	}

	TUniquePtr<FAttributeData> NewAttribute = UpgradeUtils::CreateAttribute(Owner, Prop, RowName);
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
		if (NewAttributeRaw->Modify(UpgradeData->GetModifier(NewAttributeRaw->CurrentUpgradeLevel)))
		{
			const int32 Level = NewAttributeRaw->CurrentUpgradeLevel+1;		
			NewAttributeRaw->CurrentUpgradeLevel = Level;
		}
		
		UPGRADE_DISPLAY( TEXT("%hs: Upgraded attribute %s to level %d."), __FUNCTION__, *UpgradeUtils::GetClassNameKey(NewAttributeRaw->Owner.Get()), NewAttributeRaw->CurrentUpgradeLevel);
		if (NewAttributeRaw->OnAttributeModified.IsBound())
		{
			NewAttributeRaw->OnAttributeModified.Broadcast();
		}
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
		
		const int32 Level = NewAttributeRaw->CurrentUpgradeLevel-1;		
		if (NewAttributeRaw->Modify(UpgradeData->GetModifier(Level)))
		{
			NewAttributeRaw->CurrentUpgradeLevel = Level;
		}
		if (NewAttributeRaw->OnAttributeModified.IsBound())
		{
			NewAttributeRaw->OnAttributeModified.Broadcast();
		}
	});

	AttributesByRow.FindOrAdd(RowName).Add(NewAttributeRaw);
	AttributesByKey.Add(Key, NewAttributeRaw);
	RegisteredAttributes.Add(MoveTemp(NewAttribute));

/*#if !UE_BUILD_SHIPPING
	NewAttributeRaw->OnAddModifier.Broadcast(); //Apply initial modifier
#endif*/

	UPGRADE_DISPLAY(TEXT("%hs: Bound attribute %s with row %s."), __FUNCTION__,*UpgradeUtils::GetClassNameKey(Owner), *RowName.ToString());
}

void UUpgradeComponent::Server_UpgradeByRow_Implementation(FName RowName)
{
	UPGRADE_DISPLAY(TEXT("%hs: Upgrading by row %s. Found %d Attributes"), __FUNCTION__, *RowName.ToString(), GetByRow(RowName).Num());
	for (const FAttributeData* TargetAttribute : GetByRow(RowName))
	{
		const int32 CurrentUpgradeLevel = TargetAttribute->CurrentUpgradeLevel;
		TargetAttribute->OnAddModifier.Broadcast();		
		UPGRADE_DISPLAY(TEXT("⬆️%hs: Upgraded attribute %s with row %s."), __FUNCTION__, *UpgradeUtils::GetClassNameKey(TargetAttribute->Owner.Get()), *RowName.ToString());
		if (CurrentUpgradeLevel != TargetAttribute->CurrentUpgradeLevel && OnUpgraded.IsBound())
		{
			UGameplayUtilFunctionLibrary::SendGameplayEventToActor(GetOwner(), GetOwner(), FGameplayTag::RequestGameplayTag("Event.Abilities.Upgrade"), GetOwner());
			OnUpgraded.Broadcast();
		}
	}
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
	if (const FUpgradeDisplayData* UpgradeDataPtr = InstancedStruct.GetMutablePtr<FUpgradeDisplayData>())
	{
		UPGRADE_DISPLAY(TEXT("%hs: Applying upgrade %s."), __FUNCTION__, *UpgradeDataPtr->ToString());
		const FUpgradeDisplayData& UpgradeData = *UpgradeDataPtr;	
		Server_UpgradeByRow(UpgradeData.RowName);		
	}	
	else if (const FTeamModifierData* ModifierData = InstancedStruct.GetMutablePtr<FTeamModifierData>())
	{
		UPGRADE_DISPLAY(TEXT("%hs: Applying team modifier %s."), __FUNCTION__, *ModifierData->Title.ToString());
		for (const FModifierEntry& Modifier : ModifierData->Modifiers)
		{		
			for (int i = 0; i < Modifier.TimesToApply; ++i)
			{
				if (Modifier.bShouldRemove)
				{
					DowngradeByRow(Modifier.RowName);
					continue;
				}
				
				Server_UpgradeByRow(Modifier.RowName);
			}
		} 
	}
	else
	{
		UPGRADE_ERROR(TEXT("%hs: Failed to get FUpgradeDisplayData from InstancedStruct!"), __FUNCTION__);		
	}
}

void UUpgradeComponent::Server_OnUpgradeReceived_Implementation(FInstancedStruct InstancedStruct)
{
	OnUpgradeReceived(InstancedStruct);
}

TArray<FUpgradeDisplayData> UUpgradeComponent::GetRandomUpgrades(const int32 NumberOfUpgrades)
{
	TArray<FUpgradeDisplayData> OutUpgrades;
	TArray<FAttributeUpgradeData*> UpgradeDataArrayCopy;
	if (!UpgradeDataTable)
	{
		Server_LoadDataTable(); 
		if (!UpgradeDataTable)
		{
			return OutUpgrades;
		} 
	}
	UpgradeDataTable->GetAllRows( __FUNCTION__ ,UpgradeDataArrayCopy);

	UpgradeDataArrayCopy.RemoveAll([](const FAttributeUpgradeData* Attribute)
	{
		return Attribute && Attribute->IsMatch(static_cast<int32>(EUpgradeFlag::Team));
	});

	for (int i = 1; i <= NumberOfUpgrades; ++i)
	{
		const int32 RandomIndex = FMath::RandRange(0, UpgradeDataArrayCopy.Num() - 1);
		if (!UpgradeDataArrayCopy.IsValidIndex(RandomIndex)) //Shouldn't be needed... But just in case
		{
			UPGRADE_ERROR(TEXT("%hs: RandomIndex %d is invalid!? Actual size: %d"), __FUNCTION__, RandomIndex,UpgradeDataArrayCopy.Num());
			break;
		}
		const FAttributeUpgradeData* Item = UpgradeDataArrayCopy[RandomIndex];
		OutUpgrades.Add(Item->UpgradeDisplayData);
		UpgradeDataArrayCopy.RemoveAt(RandomIndex); // To avoid duplicates
	}
	
	return OutUpgrades;	
}

FTeamModifierData UUpgradeComponent::GetTeamModifier(const FName RowName)
{
	if (!TeamModifierDataTable)
	{
		Server_LoadDataTable();
		if (!TeamModifierDataTable)
		{
			UPGRADE_ERROR(TEXT("%hs: TeamModifierDataTable is null!"), __FUNCTION__);
			return FTeamModifierData();
		}
	}
	FTeamModifierData* ModifierData = TeamModifierDataTable->FindRow<FTeamModifierData>(RowName, __FUNCTION__);
	if (!ModifierData)
	{
		UPGRADE_ERROR(TEXT("%hs: No team modifier data found for row %s!"), __FUNCTION__, *RowName.ToString());
		return FTeamModifierData();
	}
	return *ModifierData;
}

FAttributeData* UUpgradeComponent::GetByKey(UObject* Owner, FProperty* Property) const
{
	return AttributesByKey.FindRef(UpgradeUtils::GetKey(Owner, Property));
}

TArray<FUpgradeDisplayData> UUpgradeComponent::GetPlayerUpgrades()
{
	TArray<FUpgradeDisplayData> Out;

	TArray<FAttributeUpgradeData*> UpgradeDataArrayCopy;

	if (!UpgradeDataTable)
	{
		Server_LoadDataTable();
		if (!UpgradeDataTable)
		{
			return Out;
		}
	}

	UpgradeDataTable->ForeachRow<FAttributeUpgradeData>( __FUNCTION__ , [this, &Out](const FName& Key, const FAttributeUpgradeData& Value)
	{
		if (!Value.IsMatch(static_cast<int32>(EUpgradeFlag::Team)))
		{
			for (const FAttributeData* ByRow : GetByRow(Key))
			{
				FUpgradeDisplayData ValueCopy = Value.UpgradeDisplayData;
				if (ByRow)
				{
					UPGRADE_DISPLAY(TEXT("%hs: Found player attribute for row %s with current upgrade level %d."), __FUNCTION__,*Key.ToString(), ByRow->CurrentUpgradeLevel);
					ValueCopy.CurrentUpgradeLevel = ByRow->CurrentUpgradeLevel +1;  //+1 to convert from index to level
				}
				Out.Add(ValueCopy);
			}
		}
	});

	return Out;
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

void UUpgradeComponent::Server_LoadDataTable_Implementation()
{
	if (!UpgradeDataTable)
	{
		UpgradeDataTable = LoadObject<UDataTable>(nullptr, UPGRADE_TABLE_PATH);		
		
     	if (UpgradeDataTable)
     	{
     		UPGRADE_DISPLAY(TEXT("%hs: Successfully loaded UpgradeDataTable."), __FUNCTION__);
     	}
     	else
     	{
     		UPGRADE_ERROR(TEXT("%hs: Failed to load UpgradeDataTable at path: %s"), __FUNCTION__, UPGRADE_TABLE_PATH);     		
     	}		
	}
	else
	{
		UPGRADE_DISPLAY(TEXT("%hs: UpgradeDataTable already loaded."), __FUNCTION__);
	}	
	
	if (!TeamModifierDataTable)
	{
		TeamModifierDataTable = LoadObject<UDataTable>(nullptr, MODIFIER_TABLE_PATH);
		if (TeamModifierDataTable)
		{
			UPGRADE_DISPLAY(TEXT("%hs: Successfully loaded TeamModifierDataTable."), __FUNCTION__);			
		}
		else
		{
			UPGRADE_ERROR(TEXT("%hs: Failed to load TeamModifierDataTable at path: %s"), __FUNCTION__, MODIFIER_TABLE_PATH);
		}
	}
	else
	{
		UPGRADE_DISPLAY(TEXT("%hs: TeamModifierDataTable already loaded."), __FUNCTION__);
	}
}

void UUpgradeComponent::ClearAttributes([[maybe_unused]] const FString& String)
{
	UPGRADE_HI_FROM(__FUNCTION__);
	RegisteredAttributes.Empty();
	AttributesByRow.Empty();
	AttributesByKey.Empty();
	AttributesByCategory.Empty();
}
