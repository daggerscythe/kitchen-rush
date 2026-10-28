// KRMeterWidget.cpp

#include "UI/KRMeterWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UKRMeterWidget::SetMeter(float Percent, const FLinearColor& Color, const FText& LabelText)
{
	Bar->SetPercent(Percent);
	Bar->SetFillColorAndOpacity(Color);
	if (Label)
	{
		Label->SetText(LabelText);
	}
}
