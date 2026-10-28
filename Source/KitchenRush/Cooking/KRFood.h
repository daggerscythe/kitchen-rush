// KRFood.h 
// A piece of food. Whether it's been chopped and how long cooked, and turns that into a recipe part.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/KRItem.h"
#include "Cooking/KRTypes.h"
#include "KRFood.generated.h"

class UStaticMesh;
class UMaterialInterface;

UCLASS()
class KITCHENRUSH_API AKRFood : public AKRItem
{
	GENERATED_BODY()

public:
	AKRFood();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food")
	EKRFoodType FoodType = EKRFoodType::Tomato;

	/** Mesh to switch to once chopped. Leave empty for steak. */
	UPROPERTY(EditAnywhere, Category = "Food")
	TObjectPtr<UStaticMesh> ChoppedMesh;

	/** Steak only: the material slot that changes as it cooks (slot names show in the mesh's Details). */
	UPROPERTY(EditAnywhere, Category = "Food|Cooking")
	FName CookSlotName = TEXT("Steak_Meat");

	UPROPERTY(EditAnywhere, Category = "Food|Cooking")
	TObjectPtr<UMaterialInterface> CookedMaterial;

	UPROPERTY(EditAnywhere, Category = "Food|Cooking")
	TObjectPtr<UMaterialInterface> BurntMaterial;

	bool CanBeChopped() const { return FoodType != EKRFoodType::Steak && !bChopped; }
	bool CanBeCooked() const { return FoodType == EKRFoodType::Steak; }
	bool IsChopped() const { return bChopped; }
	float GetCookSeconds() const { return CookSeconds; }

	/** Switch to the chopped mesh. */
	void SetChopped();

	/** Called by the pan every frame with world seconds. */
	void AddCookTime(float WorldSeconds);

	/** What this counts as on a plate */
	FKRPlatedPart GetPlatedPart() const;

	virtual FText GetItemName() const override;
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

private:
	/** 0 raw, 1 cooked (perfect), 2 well done, 3 burnt */
	int32 GetCookStage() const;
	void UpdateCookLook();

	bool bChopped = false;
	float CookSeconds = 0.f;
	int32 ShownCookStage = 0;
};
