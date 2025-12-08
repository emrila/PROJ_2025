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
	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnClicked;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnPressed;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnReleased;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnHovered;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnUnhovered;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnFocused;

	UPROPERTY(BlueprintAssignable, Category="Events", meta = (IsBindableEvent = true))
	FOnClickableEvent OnUnfocused;

	// Blueprint-callable wrappers to broadcast the delegates
	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnClicked();

	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnPressed();

	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnReleased();

	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnHovered();

	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnUnhovered();

	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnFocused();

	UFUNCTION(BlueprintCallable, Category="Events")
	void CallOnUnfocused();
};
