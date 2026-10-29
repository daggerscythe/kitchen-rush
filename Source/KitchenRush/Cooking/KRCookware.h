// KRCookware.h
// Shared base for cookware: tick, cook while on a stove, and show a meter above them.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/KRItem.h"
#include "KRCookware.generated.h"

class UWidgetComponent;

UCLASS(Abstract)
class KITCHENRUSH_API AKRCookware : public AKRItem
{
	GENERATED_BODY()

public:
	AKRCookware();

	virtual void Tick(float DeltaSeconds) override;

	/** Sitting near one of the stove's burner markers? */
	bool IsOnBurner() const;

	/** Something thrown hit us: get knocked away (off the burner). */
	virtual void OnHitByThrownItem(AKRItem* Item) override;

protected:
	/** Called every frame with WORLD seconds while on a burner. Children do the cooking here. */
	virtual void Cook(float WorldSeconds) {}

	/** Called every frame. Children call SetMeter() here. */
	virtual void UpdateMeter() {}

	/** Show/hide and fill the floating meter. */
	void SetMeter(bool bVisible, float Percent, const FLinearColor& Color, const FText& Label);

	/** The floating meter. Set its Widget Class to WBP_CookMeter in the Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cookware")
	TObjectPtr<UWidgetComponent> Meter;
};
