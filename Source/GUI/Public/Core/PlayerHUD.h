// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "PlayerHUDInterface.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

UCLASS()
class GUI_API APlayerHUD : public AHUD, public IPlayerHUDInterface
{
	GENERATED_BODY()
	
protected:
	
	virtual void BeginPlay() override;	
	
	virtual void InitWidgets_Implementation() override;
	
	void AddGameVersionToHUD();

	template <typename T = UUserWidget>
	T* CreateAndAddToViewPort(const TSubclassOf<T>& WidgetClass, bool Visible = true);

	template <typename T = UUserWidget>
	void ToggleMenuWidget(T* Widget);

public:
	virtual void CreateAndAdd_Implementation(TSubclassOf<UUserWidget> WidgetClass, bool Visible = true) override;
	virtual void ToggleWidget_Implementation(int32 WidgetGroup) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TSubclassOf<UUserWidget> HUDClass;
	
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TObjectPtr<UUserWidget> HUDWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TSubclassOf<UUserWidget> MenuClass;
	
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TObjectPtr<UUserWidget> MenuWidget;
	
	TSharedPtr<class STextSWidget> BuildVersionSlateWidget;
	TSharedPtr<class SWeakWidget> BuildVersionWidget;
};

