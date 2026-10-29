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

	/** [M3] Launch from StartLocation with Velocity (cm/s). bFromGus marks it for the catch bonus. */
	void OnThrown(const FVector& StartLocation, const FVector& Velocity, bool bFromGus);

	bool IsHeld() const { return bHeld; }
	bool IsStored() const { return bStored; }   // [M2]
	bool IsInFlight() const { return bInFlight; } // [M3]

	/** [M3] Tossed by Gus and caught by the player before it touched anything: earns the catch bonus. */
	bool WasCaughtFromGus() const { return bWasCaughtFromGus; }

	/** [M3.7] Every trick-shot place (KRTrick bits) the player has thrown this into. */
	uint8 GetTrickShotMask() const { return TrickShotMask; }

	/** [M3.7] How many different places it was thrown into (0 to 4). */
	int32 GetTrickShotCount() const { return static_cast<int32>(FMath::CountBits(TrickShotMask)); }

	/** [M3.7] Has this ever touched the floor? Served food that did costs money. */
	bool HasTouchedGround() const { return bTouchedGround; }

	/** [M3.7] Is Hit "the floor"? A flat surface at the height of the player's feet. (Counters are 90 cm up.) */
	bool IsFloorHit(const FHitResult& Hit) const;
	UStaticMeshComponent* GetMesh() const;       // [M2]

	// ---- IKRInteractable ----
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;
	virtual bool IsCatchable() const override { return bInFlight; }   // [M3]

protected:
	virtual void BeginPlay() override;   // [M3]

	/** [M3] Children call these first in their own prompt/interact when IsInFlight(). */
	FText GetCatchPrompt(const AKRItem* HeldItem) const;
	bool CatchBy(UKRInteractionComponent* Interactor);

	/** [M3] Called at the end of OnThrown. Hazards override it (knives turn off gravity, etc.). */
	virtual void PostThrown(const FVector& Velocity) {}

	/** [M3] The first thing a flying item touches. Default: land in it if it takes us (pan, pot, plate), else bump it. */
	virtual void HandleFlightHit(AActor* OtherActor, const FHitResult& Hit);

	/** [M3] Ask OtherActor to take this item. True if it did (we may have been destroyed: return right away). */
	bool TryLandIn(AActor* OtherActor);

	/** [M3] Stop being "in flight" and let the player bump into us again. */
	void EndFlight();

	/** [M3.5] Every physics hit (flying or not) comes here first. Plates use it to shatter on the floor. */
	virtual void OnPhysicsHit(AActor* OtherActor, const FHitResult& Hit) {}

	/** [M3] Physics hit event from the mesh. Only passes the first hit of a flight on to HandleFlightHit. */
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	bool bHeld = false;
	bool bStored = false;   // [M2]
	bool bInFlight = false;          // [M3] thrown/tossed and hasn't touched anything yet
	bool bLaunchedByGus = false;     // [M3]
	bool bWasCaughtFromGus = false;  // [M3]
	uint8 TrickShotMask = 0;         // [M3.7] replaces M3.5's single bThrownIntoPlace
	bool bTouchedGround = false;     // [M3.7]
};
