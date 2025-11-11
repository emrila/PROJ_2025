// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "AttributeData.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttributeModified);

USTRUCT(BlueprintType)
struct FAttributeData
{
	GENERATED_BODY()
	FAttributeData () = default;
	FAttributeData (UObject* InOwner, FProperty* InProperty, const FName InRowName, const float InInitialValue)
		: Owner(InOwner), Property(InProperty), RowName(InRowName), InitialValue(InInitialValue)
	{
	}

	TWeakObjectPtr<UObject> Owner;
	FProperty* Property = nullptr;

	FName RowName; 
	int32 CurrentUpgradeLevel = 1;
	float InitialValue = 0.f;
	TVariant<float, int32> CurrentValue;
	FOnAttributeModified OnAttributeModified;	
};

USTRUCT()
struct FAttributeBase
{
	GENERATED_BODY()
	FAttributeBase () = default;
	FAttributeBase (UObject* InOwner, FProperty* InProperty, const FName InRowName, const float InInitialValue)
		: Owner(InOwner), Property(InProperty), RowName(InRowName)
	{
	}

	TWeakObjectPtr<UObject> Owner = nullptr;
	FProperty* Property = nullptr;

	FName RowName; 
	int32 CurrentUpgradeLevel = 1;
	/*float InitialValue = 0.f;
	TVariant<float, int32> CurrentValue;*/
	FOnAttributeModified OnAttributeModified;

	bool IsValidProperty() const
	{
		return Property && (
			Property->IsA<FFloatProperty>() ||
			Property->IsA<FIntProperty>()
		);
	}	

	template<typename V = FProperty, typename T>
	T GetValueFromContainer() const
	{
		V* Prop = CastFieldChecked<V>(Property);
		return Prop->GetPropertyValue_InContainer(Owner);		
	}
	
	template<typename T>
	void SetValueInContainer(const T& InValue)
	{
		if (!IsValidProperty())
		{
			return;
		}
		const T* Prop = CastFieldChecked<T>(Property);
		Prop->SetPropertyValue_InContainer(Owner, InValue);
	}	
};

USTRUCT(BlueprintType)
struct FAttributeFloat : public FAttributeBase
{
	GENERATED_BODY()
	
	FAttributeFloat () = default;
	FAttributeFloat (UObject* InOwner, FProperty* InProperty, const FName InRowName, const float InInitialValue) : FAttributeBase(InOwner, InProperty, InRowName, InInitialValue)
	{
	}
	
	float InitialValue = 0.f;
	//float LastValue = 0.f;
	
};

USTRUCT(BlueprintType)
struct FDependentAttribute
{
	GENERATED_BODY()

	FDependentAttribute() = default;
	FDependentAttribute(UObject* InOwner, FProperty* InProperty, bool InOverrideOnModified)
		: Owner(InOwner), Property(InProperty), bOverrideOnModified(InOverrideOnModified)
	{
	}
	TWeakObjectPtr<UObject> Owner;
	FProperty* Property = nullptr;
	bool bOverrideOnModified = false; // Ifall nuvarande värde ska spegla det modifierade värdet. Om inte så kommer den ändras procentuellt.
};
