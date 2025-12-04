// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class GUI_API STextSWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextSWidget){}
	SLATE_ARGUMENT(FText, InTextContent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};