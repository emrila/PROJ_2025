// 🐲Furkan approves of this🐲


#include "Core/PlayerHUD.h"

#include "Dev/GUILog.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Slate/STextSWidget.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();
	
	InitWidgets();
	AddGameVersionToHUD();
}

void APlayerHUD::InitWidgets()
{
	if (HUDClass)
	{
		HUDWidget = CreateAndAddToViewPort<UUserWidget>(HUDClass);
	}
	if (MenuClass)
	{
		MenuWidget = CreateAndAddToViewPort<UUserWidget>(MenuClass, false);
	}
}

void APlayerHUD::AddGameVersionToHUD()
{
	if (!GEngine)
	{
		GUI_ERROR( TEXT("%hs: GEngine is null!"), __FUNCTION__);
		return;
	}
	
	FString ProjectVersion;
	
	const TCHAR* Section = TEXT("/Script/EngineSettings.GeneralProjectSettings");
	const TCHAR* Key = TEXT("ProjectVersion");	
	GConfig->GetString(Section,Key, ProjectVersion, GGameIni);

	FString VersionString;
#if UE_BUILD_SHIPPING //include the build date
	
	FDateTime Now = FDateTime::Now();
	FString BuildDate = Now.ToString(TEXT("%Y.%m.%d %H:%M:%S"));
	VersionString = FString::Printf(TEXT("%s\n%s"), *BuildDate, *ProjectVersion);
#else // Just show the version	
	VersionString = FString::Printf(TEXT("Version: %s"), *ProjectVersion);
#endif

	BuildVersionSlateWidget = SNew(STextSWidget).InTextContent(FText::FromString(VersionString));

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(BuildVersionWidget, SWeakWidget)
		.PossiblyNullContent(BuildVersionSlateWidget.ToSharedRef()));
	
}
