// KRReceiptLineWidget.cpp

#include "UI/KRReceiptLineWidget.h"
#include "Components/TextBlock.h"

void UKRReceiptLineWidget::Setup(const FKRReceiptLine& Line)
{
	LineText->SetText(Line.Text);

	FLinearColor Color = BonusColor;
	switch (Line.Kind)
	{
	case EKRReceiptLine::Header:  Color = HeaderColor; break;
	case EKRReceiptLine::Penalty: Color = PenaltyColor; break;
	case EKRReceiptLine::Total:   Color = TotalColor; break;
	default:                      Color = BonusColor; break;
	}
	LineText->SetColorAndOpacity(FSlateColor(Color));

	// Indent the bonus/penalty lines. A render translation moves the drawing without changing the layout.
	const bool bIndented = Line.Kind == EKRReceiptLine::Bonus || Line.Kind == EKRReceiptLine::Penalty;
	LineText->SetRenderTranslation(FVector2D(bIndented ? Indent : 0.f, 0.f));
}
