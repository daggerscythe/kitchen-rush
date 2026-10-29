// KRPan.h 
// Holds one steak. raw -> perfect -> well done -> burnt.

#pragma once

#include "CoreMinimal.h"
#include "Cooking/KRCookware.h"
#include "Cooking/KRTypes.h" // [M3.7] KRTrick
#include "KRPan.generated.h"

class AKRFood;
class AKRHazardZone;

UCLASS()
class KITCHENRUSH_API AKRPan : public AKRCookware
{
	GENERATED_BODY()

public:
	AKRPan();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	// [M3] A steak thrown at the pan (or caught while holding it) lands in it.
	virtual bool CanReceiveItem(const AKRItem* Item) const override;
	virtual bool ReceiveThrownItem(AKRItem* Item) override;
	virtual uint8 GetTrickShotFlag() const override { return KRTrick::Pan; } // [M3.7]

protected:
	virtual void Cook(float WorldSeconds) override;
	virtual void UpdateMeter() override;
	virtual void Destroyed() override;

	/** Where the steak sits. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pan")
	TObjectPtr<USceneComponent> FoodSlot;

	/** [M3] Spawned when a steak is left too long: set to BP_GreaseFire. (Empty = an invisible fire, still hurts.) */
	UPROPERTY(EditAnywhere, Category = "Pan")
	TSubclassOf<AKRHazardZone> GreaseFireClass;

private:
	/** [M3] Put a steak in the pan. */
	void AddSteak(AKRFood* Food);
	void StartGreaseFire();

	UPROPERTY()
	TObjectPtr<AKRFood> Steak;
};
