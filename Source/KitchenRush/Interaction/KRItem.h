// KRItem.h
// Anything the player can carry: food, plates, bowls (and in M3 the pan and pot) or that is stored in a container

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bCanBePickedUp = true;

	virtual FText GetItemName() const { return DisplayName; }

	/** Attach to the player's hold point and switch off physics. */
	void OnPickedUp(USceneComponent* HoldPoint);

	/** Detach, move to a spot in the world, and switch physics back on. */
	void OnPlaced(const FVector& Location, const FRotator& Rotation);

	/** Put inside a container: no physics, no collision, attached at an offset from Parent. */
	void OnStored(USceneComponent* Parent, const FVector& RelativeLocation);

	bool IsHeld() const { return bHeld; }
	bool IsStored() const { return bStored; }   // [M2]
	UStaticMeshComponent* GetMesh() const;       // [M2]

	// ---- IKRInteractable ----
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	bool bHeld = false;
	bool bStored = false;   // [M2]
};
