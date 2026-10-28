// KRMeterWidget.h 
// The little bar floating over the pan and pot. WBP_CookMeter is its Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRMeterWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS(Abstract)
class KITCHENRUSH_API UKRMeterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMeter(float Percent, const FLinearColor& Color, const FText& LabelText);

protected:
	/** Must be a Progress Bar named Bar. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Bar;

	/** Optional Text named Label, e.g. "Perfect Steak". */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Label;
};
