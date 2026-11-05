// KRPan.h 
// Holds one steak. raw -> perfect -> well done -> burnt.

#pragma once

#include "CoreMinimal.h"
#include "Cooking/KRCookware.h"
#include "Cooking/KRTypes.h"
#include "KRPan.generated.h"

class AKRFood;
class AKRHazardZone;

UCLASS()
class KITCHENRUSH_API AKRPan : public AKRCookware
{
	GENERATED_BODY()

public:
	AKRPan();

	/** A steak is in the pan and the pan is on a burner. */
	bool IsCookingSteak() const { return Steak != nullptr && IsOnBurner(); }
	/** The crosshair ray this frame (called by the player's interaction while it looks at the pan).
	 *  If the ray passes over the steak, E and the prompt act on the steak; otherwise on the pan. */
	void UpdateAim(const FVector& RayStart, const FVector& RayDirection);

	/** Placed with E, the pan turns this many degrees from the way you face. -90 = handle to your left. */
	virtual float GetPlaceYawOffset() const override { return PlaceYawOffset; }

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	// A steak thrown at the pan (or caught while holding it) lands in it.
	virtual bool CanReceiveItem(const AKRItem* Item) const override;
	virtual bool ReceiveThrownItem(AKRItem* Item) override;
	virtual uint8 GetTrickShotFlag() const override { return KRTrick::Pan; }
	virtual EKRSound GetImpactSound() const override { return EKRSound::PanClang; }

protected:
	virtual void Cook(float WorldSeconds) override;
	virtual void UpdateMeter() override;
	virtual void UpdateCookSound() override;
	virtual void Destroyed() override;

	/** Where the steak sits. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pan")
	TObjectPtr<USceneComponent> FoodSlot;

	/** Spawned when a steak is left too long: set to BP_GreaseFire. (Empty = an invisible fire, still hurts.) */
	UPROPERTY(EditAnywhere, Category = "Pan")
	TSubclassOf<AKRHazardZone> GreaseFireClass;

	/** -90 = the handle points to your left when you place the pan. Try 90 if it ends up on your right. */
	UPROPERTY(EditAnywhere, Category = "Pan")
	float PlaceYawOffset = -90.f;

private:
	/** Put a steak in the pan. */
	void AddSteak(AKRFood* Food);
	void StartGreaseFire();

	UPROPERTY()
	TObjectPtr<AKRFood> Steak;
	bool bAimingAtSteak = false;
};
