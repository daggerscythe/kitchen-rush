// KRPot.h 
// Once all ingr are in and pot is on burner, it simmers
// ready at 12 s, starts boiling over at 18 s, ruined at 24 s
#pragma once

#include "CoreMinimal.h"
#include "Cooking/KRCookware.h"
#include "KRPot.generated.h"

UCLASS()
class KITCHENRUSH_API AKRPot : public AKRCookware
{
	GENERATED_BODY()

public:
	AKRPot();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	bool IsSoupReady() const;

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

private:
	void ResetPot();
	void UpdateSurface();

	int32 TomatoCount = 0;
	float SimmerSeconds = 0.f;
};
