#pragma once

DECLARE_LOG_CATEGORY_EXTERN(CLogGUI, Log, All);

#define GUI_LOG(Severity, Text, ...) UE_LOG(CLogGUI, Severity, Text, ##__VA_ARGS__)
#define GUI_DISPLAY(Text, ...) UE_LOG(CLogGUI, Display, Text, ##__VA_ARGS__)
#define GUI_WARNING(Text, ...) UE_LOG(CLogGUI, Warning, Text, ##__VA_ARGS__)
#define GUI_ERROR(Text, ...) UE_LOG(CLogGUI, Error, Text, ##__VA_ARGS__)

#define GUI_HI_FROM(...) UE_LOG(CLogGUI, Display, TEXT("🖥️%hs"), ##__VA_ARGS__)