#pragma once

//-------- Upgrade Log Category --------//
DECLARE_LOG_CATEGORY_EXTERN(CLogUpgrade, Log, All);

#define UPGRADE_LOG(Severity, Text, ...) UE_LOG(CLogUpgrade, Severity, Text, ##__VA_ARGS__)
#define UPGRADE_DISPLAY(Text, ...) UE_LOG(CLogUpgrade, Display, Text, ##__VA_ARGS__)
#define UPGRADE_WARNING(Text, ...) UE_LOG(CLogUpgrade, Warning, Text, ##__VA_ARGS__)
#define UPGRADE_ERROR(Text, ...) UE_LOG(CLogUpgrade, Error, Text, ##__VA_ARGS__)

#define UPGRADE_HI_FROM(...) UE_LOG(CLogUpgrade, Display, TEXT("%hs: Called"), ##__VA_ARGS__)

//-------- Spawn Log Category --------//
DECLARE_LOG_CATEGORY_EXTERN(CLogUpgradeSpawn, Log, All);

#define UPGRADE_SPAWN_LOG(Severity, Text, ...) UE_LOG(CLogUpgradeSpawn, Severity, Text, ##__VA_ARGS__)
#define UPGRADE_SPAWN_DISPLAY(Text, ...) UE_LOG(CLogUpgradeSpawn, Display, Text, ##__VA_ARGS__)
#define UPGRADE_SPAWN_WARNING(Text, ...) UE_LOG(CLogUpgradeSpawn, Warning, Text, ##__VA_ARGS__)
#define UPGRADE_SPAWN_ERROR(Text, ...) UE_LOG(CLogUpgradeSpawn, Error, Text, ##__VA_ARGS__)

#define UPGRADE_SPAWN_HI_FROM(...) UE_LOG(CLogUpgradeSpawn, Display, TEXT("%hs: Called"), ##__VA_ARGS__)
