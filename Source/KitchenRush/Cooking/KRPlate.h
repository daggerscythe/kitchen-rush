// KRPlate.h 
// A plate or a bowl. Plates hold up to MaxPlateItems pieces of food;
// bowls hold one serving of soup.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/KRItem.h"
#include "Cooking/KRTypes.h"
#include "KRPlate.generated.h"

class AKRFood;

UCLASS()
class KITCHENRUSH_API AKRPlate : public AKRItem
{
	GENERATED_BODY()

public:
	AKRPlate();

	/** Bowls only take soup (from the pot); plates take anything else. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	bool bIsBowl = false;

	bool CanAcceptFood(const AKRFood* Food) const;
	bool AddFood(AKRFood* Food);

	bool CanAcceptSoup() const;
	void AddSoup(bool bPerfect);

	/** Destroys everything on the plate (used by the trash). */
	void ClearContents();
	bool IsEmpty() const;

	/** Everything on the plate as recipe parts, for scoring. */
	TArray<FKRPlatedPart> GetPlatedParts() const;

	/** "Chopped Tomato, Perfect Steak" or "empty". */
	FText GetContentsText() const;

	virtual FText GetItemName() const override;
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	/** Bowl only: the soup surface, hidden until filled. Pick its mesh in BP_Bowl. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<UStaticMeshComponent> SoupFill;

	/** When the plate is destroyed (served or trashed), destroy the food on it too. */
	virtual void Destroyed() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<AKRFood>> Foods;

	bool bHasSoup = false;
	bool bSoupPerfect = false;
};
