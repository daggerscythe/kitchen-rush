// KRTicketWidget.h
// One order ticket on the HUD. WBP_Ticket is its Blueprint; the HUD makes one per open order.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRTicketWidget.generated.h"

struct FKRRecipeRow;
class UImage;
class UProgressBar;
class UTextBlock;

UCLASS(Abstract)
class KITCHENRUSH_API UKRTicketWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fill in the dish name, icon and parts. Called once when the ticket appears. */
	void Setup(const FKRRecipeRow& Recipe);

	/** 1 = full patience, 0 = customer leaves. [M4] Below 0 hides the bar (a ticket with no patience). */
	void SetPatience(float Fraction);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DishName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PatienceBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DishIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PartsText;
};
