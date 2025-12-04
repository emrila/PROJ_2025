// 🐲Furkan approves of this🐲


#include "Widgets/Slate/STextSWidget.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STextSWidget::Construct(const FArguments& InArgs)
{
	const FText TextContent = InArgs._InTextContent;

	ChildSlot
	[
		SNew(SOverlay)+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		[
			SNew(STextBlock).Text(TextContent)
		]
	];
	
	SetVisibility(EVisibility::HitTestInvisible);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION