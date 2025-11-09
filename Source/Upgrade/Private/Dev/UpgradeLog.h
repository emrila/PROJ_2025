#pragma once

DECLARE_LOG_CATEGORY_EXTERN(CLogUpgrade, Log, All);

#define UPGRADE_LOG(Severity, Text, ...) UE_LOG(CLogUpgrade, Severity, Text, ##__VA_ARGS__)
#define UPGRADE_DISPLAY(Text, ...) UE_LOG(CLogUpgrade, Display, Text, ##__VA_ARGS__)
#define UPGRADE_WARNING(Text, ...) UE_LOG(CLogUpgrade, Warning, Text, ##__VA_ARGS__)
#define UPGRADE_ERROR(Text, ...) UE_LOG(CLogUpgrade, Error, Text, ##__VA_ARGS__)

#define UPGRADE_HI_FROM(...) UE_LOG(CLogUpgrade, Display, TEXT("%hs: Called"), ##__VA_ARGS__)
