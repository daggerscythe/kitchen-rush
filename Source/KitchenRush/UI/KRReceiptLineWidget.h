// KRReceiptLineWidget.h
// [M3.7] One line of an order receipt in the HUD's receipt log ("+$20  Nana's Tomato Soup",
// "- Perfect cook! (+$2)", "Total: $27"). WBP_ReceiptLine is its Blueprint: just one Text called LineText.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Cooking/KRTypes.h"
#include "KRReceiptLineWidget.generated.h"

class UTextBlock;

UCLASS(Abstract)
class KITCHENRUSH_API UKRReceiptLineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the words and colour. Bonus/penalty lines are indented under the header. */
	void Setup(const FKRReceiptLine& Line);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LineText;

	UPROPERTY(EditAnywhere, Category = "Receipt")
	FLinearColor HeaderColor = FLinearColor(1.f, 0.95f, 0.85f);

	UPROPERTY(EditAnywhere, Category = "Receipt")
	FLinearColor BonusColor = FLinearColor(0.45f, 0.9f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Receipt")
	FLinearColor PenaltyColor = FLinearColor(1.f, 0.45f, 0.45f);

	UPROPERTY(EditAnywhere, Category = "Receipt")
	FLinearColor TotalColor = FLinearColor(1.f, 0.84f, 0.42f);

	/** How far (px) bonus and penalty lines are pushed in from the left. */
	UPROPERTY(EditAnywhere, Category = "Receipt")
	float Indent = 28.f;
};
