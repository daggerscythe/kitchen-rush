// KRChoppingBoard.h
// Put a tomato, lettuce or onion on the board, then press E (with empty hands) to chop.
// Each chop is an action.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/KRInteractable.h"
#include "Cooking/KRTypes.h" // KRTrick
#include "KRChoppingBoard.generated.h"

class AKRFood;
class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRChoppingBoard : public AActor, public IKRInteractable
{
	GENERATED_BODY()

public:
	AKRChoppingBoard();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	// A whole tomato/lettuce/onion thrown onto an empty board lands on it, ready to chop.
	virtual bool CanReceiveItem(const AKRItem* Item) const override;
	virtual bool ReceiveThrownItem(AKRItem* Item) override;
	virtual uint8 GetTrickShotFlag() const override { return KRTrick::Board; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Where food sits on the board. Move it in BP_ChoppingBoard if food floats or sinks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<USceneComponent> FoodSlot;

private:
	UPROPERTY()
	TObjectPtr<AKRFood> Food;

	int32 Chops = 0;
};
