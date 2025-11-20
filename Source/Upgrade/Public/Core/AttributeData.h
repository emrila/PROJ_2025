#pragma once

#include "CoreMinimal.h"
#include "AttributeData.generated.h"

struct FAttributeFloat;
struct FAttributeInt32;

DECLARE_MULTICAST_DELEGATE(FOnAttributeModified);

USTRUCT(BlueprintType)
struct FModiferData
{
	GENERATED_BODY()
	FModiferData() = default;

	FModiferData(const float InMultiplier, const bool bInRemoveModifier) : Multiplier(InMultiplier), bRemoveModifier(bInRemoveModifier)	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "Used as: value * Multiplier"))
	float Multiplier = 0.1f;
	
	bool bRemoveModifier = false;

	template <typename T>
	T ApplyModifier(const T& BaseValue) const
	{
		if (bRemoveModifier)
		{
			return BaseValue /*- BaseValue */* Multiplier;
		}
		return BaseValue /*+ BaseValue */* Multiplier;
	}	
};

USTRUCT()
struct FAttributeBase
{
	GENERATED_BODY()
	virtual ~FAttributeBase() = default;
	FAttributeBase() = default;

	FAttributeBase(UObject* InOwner, FProperty* InProperty, const FName InRowName)
		: Owner(InOwner), Property(InProperty), RowName(InRowName)
	{
	}

	TWeakObjectPtr<UObject> Owner = nullptr;
	FProperty* Property = nullptr;

	FName RowName;
	int32 CurrentUpgradeLevel = 1;

	FOnAttributeModified OnAttributeModified;
	FOnAttributeModified OnRemoveModifier;
	FOnAttributeModified OnAddModifier;

	static bool IsValidProperty(const FProperty* Property);

	virtual void Modify(const FModiferData ModifierData){}

	template <typename T, class PropType>
	T GetValueFromContainer() const;

	template <typename T, class PropType>
	void SetValueInContainer(const T& InValue);

	template <typename T, class PropType>
	bool ModifyContainer(const FModiferData ModifierData);
};

template <typename T, class PropType>
T FAttributeBase::GetValueFromContainer() const
{
	PropType* Prop = CastFieldChecked<PropType>(Property);
	return Prop->GetPropertyValue_InContainer(Owner.Get());
}

template <typename T, class PropType>
void FAttributeBase::SetValueInContainer(const T& InValue)
{
	if (!IsValidProperty(Property))
	{
		return;
	}
	PropType* Prop = CastFieldChecked<PropType>(Property);
	Prop->SetPropertyValue_InContainer(Owner.Get(), InValue);
}

template <typename T, class PropType>
bool FAttributeBase::ModifyContainer(const FModiferData ModifierData)
{
	const T NewValue = ModifierData.ApplyModifier<T>(GetValueFromContainer<T, PropType>());
	SetValueInContainer<T, PropType>(NewValue);

	if (NewValue != GetValueFromContainer<T, PropType>())
	{
		return false;
	}
	if (ModifierData.bRemoveModifier)
	{
		CurrentUpgradeLevel--;
	}
	else
	{
		CurrentUpgradeLevel++;
	}
	return true;
}

USTRUCT(BlueprintType)
struct FAttributeFloat : public FAttributeBase
{
	GENERATED_BODY()

	FAttributeFloat() = default;
	FAttributeFloat(UObject* InOwner, FProperty* InProperty, const FName InRowName) : FAttributeBase(InOwner, InProperty, InRowName)
	{
		InitialValue = GetValueFromContainer<float, FFloatProperty>();
	}

	virtual void Modify(FModiferData ModifierData) override;
	float InitialValue = 0.f;
};

USTRUCT(BlueprintType)
struct FAttributeInt32 : public FAttributeBase
{
	GENERATED_BODY()

	FAttributeInt32() = default;
	FAttributeInt32(UObject* InOwner, FProperty* InProperty, const FName InRowName) : FAttributeBase(InOwner, InProperty, InRowName)
	{
		InitialValue = GetValueFromContainer<int32, FIntProperty>();
	}

	virtual void Modify(FModiferData ModifierData) override;
	int32 InitialValue = 0.f;
};
