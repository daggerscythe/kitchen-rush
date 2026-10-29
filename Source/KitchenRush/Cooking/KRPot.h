// KRPot.h 
// Once all ingr are in and pot is on burner, it simmers
// ready at 12 s, starts boiling over at 18 s, ruined at 24 s
#pragma once

#include "CoreMinimal.h"
#include "Cooking/KRCookware.h"
#include "Cooking/KRTypes.h" // [M3.7] KRTrick
#include "KRPot.generated.h"

class AKRHazardZone;

UCLASS()
class KITCHENRUSH_API AKRPot : public AKRCookware
{
	GENERATED_BODY()

public:
	AKRPot();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	bool IsSoupReady() const;

	// [M3] A chopped tomato thrown at the pot (or caught while holding it) goes in.
	virtual bool CanReceiveItem(const AKRItem* Item) const override;
	virtual bool ReceiveThrownItem(AKRItem* Item) override;
	virtual uint8 GetTrickShotFlag() const override { return KRTrick::Pot; } // [M3.7]

protected:
	virtual void Cook(float WorldSeconds) override;
	virtual void UpdateMeter() override;

	/** The soup surface that rises as tomatoes go in. Pick SM_SoupPot_Surface in BP_Pot. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pot")
	TObjectPtr<UStaticMeshComponent> SoupSurface;

	/** Surface height (cm above the pot's pivot) when empty and when full. */
	UPROPERTY(EditAnywhere, Category = "Pot")
	float SurfaceEmptyZ = 3.f;

	UPROPERTY(EditAnywhere, Category = "Pot")
	float SurfaceFullZ = 17.f;

	/** [M3] Spawned when the soup boils over: set to BP_Spill. (Empty = an invisible spill, still hurts.) */
	UPROPERTY(EditAnywhere, Category = "Pot")
	TSubclassOf<AKRHazardZone> SpillClass;

private:
	void ResetPot();
	void UpdateSurface();
	void AddTomato(AKRItem* Tomato); // [M3]
	void BoilOver();                 // [M3]

	int32 TomatoCount = 0;
	FKRPlatedPart TomatoTally; // [M3.7] trick shots, Gus Assists and ground touches of every tomato that went in
	float SimmerSeconds = 0.f;
};
