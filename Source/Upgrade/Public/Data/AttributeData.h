#pragma once

#include "CoreMinimal.h"
#include "Data/ModifierData.h"
#include "AttributeData.generated.h"

struct FAttributeFloat;
struct FAttributeInt32;

DECLARE_MULTICAST_DELEGATE(FOnAttributeModified);


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
	int32 CurrentUpgradeLevel = 0;

	FOnAttributeModified OnAttributeModified;
	FOnAttributeModified OnRemoveModifier;
	FOnAttributeModified OnAddModifier;

	static bool IsValidProperty(const FProperty* Property);

	virtual bool Modify(const FModiferData ModifierData)
	{
		return false;
	}

	template <typename T, class PropType>
	T GetValueFromContainer() const;

	template <typename T, class PropType>
	void SetValueInContainer(const T& InValue);

	template <typename T, class PropType>
	bool AttemptModifyContainer(const FModiferData ModifierData);
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
bool FAttributeBase::AttemptModifyContainer(const FModiferData ModifierData)
{
	const T NewValue = ModifierData.ApplyModifier<T>(GetValueFromContainer<T, PropType>());
	SetValueInContainer<T, PropType>(NewValue);

	if (NewValue != GetValueFromContainer<T, PropType>())
	{
		return false;
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

	virtual bool Modify(FModiferData ModifierData) override;
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

	virtual bool Modify(FModiferData ModifierData) override;
	int32 InitialValue = 0.f;
};
