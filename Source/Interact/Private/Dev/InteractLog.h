#pragma once

DECLARE_LOG_CATEGORY_EXTERN(CLogInteract, Log, All);

#define INTERACT_LOG(Severity, Text, ...) UE_LOG(CLogInteract, Severity, Text, ##__VA_ARGS__)
#define INTERACT_DISPLAY(Text, ...) UE_LOG(CLogInteract, Display, Text, ##__VA_ARGS__)
#define INTERACT_WARNING(Text, ...) UE_LOG(CLogInteract, Warning, Text, ##__VA_ARGS__)
#define INTERACT_ERROR(Text, ...) UE_LOG(CLogInteract, Error, Text, ##__VA_ARGS__)

#define INTERACT_HI_FROM(...) UE_LOG(CLogInteract, Display, TEXT("%hs: Called"), ##__VA_ARGS__)
