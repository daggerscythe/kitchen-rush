// KRTypes.h
// Small shared types: food kinds, recipe parts, the recipe table row, open order

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KRTypes.generated.h"

class UTexture2D;

/** The four raw crate ingredients */
UENUM(BlueprintType)
enum class EKRFoodType : uint8
{
	Tomato,
	Lettuce,
	Onion,
	Steak
};

/** One thing in a recipe+ its the state it has to be in */
UENUM(BlueprintType)
enum class EKRDishPart : uint8
{
	None           UMETA(DisplayName = "Wrong item"),
	ChoppedTomato  UMETA(DisplayName = "Chopped Tomato"),
	ChoppedLettuce UMETA(DisplayName = "Chopped Lettuce"),
	ChoppedOnion   UMETA(DisplayName = "Chopped Onion"),
	CookedSteak    UMETA(DisplayName = "Cooked Steak"),
	TomatoSoup     UMETA(DisplayName = "Tomato Soup")
};

/** One row of the DT_Recipes Data Table. The row name is the ID. */
USTRUCT(BlueprintType)
struct FKRRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TObjectPtr<UTexture2D> Icon;

	/** What must be on the plate. Order doesn't matter; repeats are allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TArray<EKRDishPart> Parts;

	/** Soup goes in a bowl; everything else on a plate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	bool bServedInBowl = false;

	/** Number of steps in the dish. Patience = PatienceBase + PatiencePerStep x Steps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 Steps = 2;

	/** Can this dish come up as a random order? (Plain Steak is tutorial-only.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	bool bInRandomPool = true;
};

/** One part sitting on a plate or in a bowl. */
USTRUCT(BlueprintType)
struct FKRPlatedPart
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Plate")
	EKRDishPart Part = EKRDishPart::None;

	/** True for parts that are cooked (steak, soup), which can earn the perfect-cook bonus. */
	UPROPERTY(BlueprintReadOnly, Category = "Plate")
	bool bCooked = false;

	/** Cooked parts only: pulled off the heat inside the perfect window. */
	UPROPERTY(BlueprintReadOnly, Category = "Plate")
	bool bPerfect = false;
};

/** An open ticket on the rail. */
USTRUCT(BlueprintType)
struct FKROrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	int32 Id = 0;

	/** Row name in DT_Recipes. */
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FName RecipeRow;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	float PatienceTotal = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	float PatienceLeft = 0.f;
};
