// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(CLogSelection, Log, All);

#define SELECTION_LOG(Severity, Text, ...) UE_LOG(CLogSelection, Severity, Text, ##__VA_ARGS__)
#define SELECTION_DISPLAY(Text, ...) UE_LOG(CLogSelection, Display, Text, ##__VA_ARGS__)
#define SELECTION_WARNING(Text, ...) UE_LOG(CLogSelection, Warning, Text, ##__VA_ARGS__)
#define SELECTION_ERROR(Text, ...) UE_LOG(CLogSelection, Error, Text, ##__VA_ARGS__)

#define SELECTION_HI_FROM(...) UE_LOG(CLogSelection, Display, TEXT("💌%hs"), ##__VA_ARGS__)
