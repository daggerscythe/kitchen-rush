// KRItem.h
// Anything the player can carry: tomato, lettuce, onion, steak, etc

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/KRInteractable.h"
#include "KRItem.generated.h"

class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRItem : public AActor, public IKRInteractable
{
	GENERATED_BODY()

public:
	AKRItem();

	/** Name used in prompts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** Attach to the player's hold point and switch off physics */
	void OnPickedUp(USceneComponent* HoldPoint);

	/** Detach, move to a spot in the world, and switch physics back on */
	void OnPlaced(const FVector& Location, const FRotator& Rotation);

	bool IsHeld() const { return bHeld; }

	// ---- IKRInteractable ----
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	/** The visible model */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	bool bHeld = false;
};
