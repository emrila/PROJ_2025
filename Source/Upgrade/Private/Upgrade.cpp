#include "Upgrade.h"

#include "Util/UpgradeLog.h"

#define LOCTEXT_NAMESPACE "FUpgradeModule"


void FUpgradeModule::StartupModule()
{
    UPGRADE_DISPLAY(TEXT("Upgrade module has started!") );
}

void FUpgradeModule::ShutdownModule()
{
    UPGRADE_DISPLAY(TEXT("Shutdown module has started!") );
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUpgradeModule, Upgrade)
