// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClickableBase.generated.h"

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClickableEvent);

UCLASS(BlueprintType, Blueprintable)
class GUI_API UClickableBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called when the clickable element is clicked */
	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnClicked;

	/** Called when the clickable element is pressed */
	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnPressed;

	/** Called when the clickable element is released */
	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnReleased;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnHovered;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnUnhovered;
};
