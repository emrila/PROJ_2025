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

	FName RowName; //Måste vara samma som i "row name" som sätts i själva datatabellen.
	int32 CurrentUpgradeLevel = 1;
	float InitialValue = 0.f;

	FOnAttributeModified OnAttributeModified;
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
