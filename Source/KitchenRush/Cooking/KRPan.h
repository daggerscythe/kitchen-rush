// KRPan.h 
// Holds one steak. raw -> perfect -> well done -> burnt.

#pragma once

#include "CoreMinimal.h"
#include "Cooking/KRCookware.h"
#include "KRPan.generated.h"

class AKRFood;

UCLASS()
class KITCHENRUSH_API AKRPan : public AKRCookware
{
	GENERATED_BODY()

public:
	AKRPan();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	virtual void Cook(float WorldSeconds) override;
	virtual void UpdateMeter() override;
	virtual void Destroyed() override;

	/** Where the steak sits. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pan")
	TObjectPtr<USceneComponent> FoodSlot;

private:
	UPROPERTY()
	TObjectPtr<AKRFood> Steak;
};
