// KRTypes.h
// Small shared types: food kinds, recipe parts, the recipe table row, open order

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KRTypes.generated.h"

class UTexture2D;

/** [M3.7] Trick-shot places, as bits: an ingredient remembers each place it was THROWN into (once per place). */
namespace KRTrick
{
	constexpr uint8 Board = 1 << 0;
	constexpr uint8 Plate = 1 << 1;
	constexpr uint8 Pan   = 1 << 2;
	constexpr uint8 Pot   = 1 << 3;
}

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

	/** [M3.7] Full price of a 100% correct dish, in $. 0 = use Tip Base from DA_Tuning. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 Price = 0;
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

	// [M3.7] Counts, so soup can add up both of its tomatoes.

	/** Trick shots: one per place (board, plate, pan, pot) each ingredient was thrown into. */
	UPROPERTY(BlueprintReadOnly, Category = "Plate")
	int32 TrickShots = 0;

	/** Ingredients caught from Gus that never touched the ground. */
	UPROPERTY(BlueprintReadOnly, Category = "Plate")
	int32 GusAssists = 0;

	/** Ingredients that touched the floor at some point. */
	UPROPERTY(BlueprintReadOnly, Category = "Plate")
	int32 GroundTouches = 0;
};

/** [M3.7] What a line of the order receipt is, so the HUD can colour it. */
UENUM(BlueprintType)
enum class EKRReceiptLine : uint8
{
	Header,   // "+$20  Nana's Tomato Soup"
	Bonus,    // "Perfect cook! (+$2)"
	Penalty,  // "1 item touched the ground (-$3)"
	Total     // "Total: $27"
};

/** [M3.7] One line of an order receipt. */
USTRUCT(BlueprintType)
struct FKRReceiptLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Receipt")
	EKRReceiptLine Kind = EKRReceiptLine::Bonus;

	UPROPERTY(BlueprintReadOnly, Category = "Receipt")
	FText Text;
};

/** [M3.7] A served order's receipt: shown line by line in the HUD's receipt log. */
USTRUCT(BlueprintType)
struct FKRReceipt
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Receipt")
	int32 Id = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Receipt")
	TArray<FKRReceiptLine> Lines;
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
