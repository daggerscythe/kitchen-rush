// KRTrash.h
// Throw away food, or empty the plate/bowl in your hands.
// [M3.5] The lid is part of the bin now: it opens when you look at the bin with something to throw away,
// and pops open when something is thrown in.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/KRInteractable.h"
#include "KRTrash.generated.h"

class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRTrash : public AActor, public IKRInteractable
{
	GENERATED_BODY()

public:
	AKRTrash();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	// [M3] Food or a hot sauce bottle thrown into the bin is gone. (Plates, pans and pots bounce off.)
	virtual bool CanReceiveItem(const AKRItem* Item) const override;
	virtual bool ReceiveThrownItem(AKRItem* Item) override;

	virtual void Tick(float DeltaSeconds) override; // [M3.5] animates the lid

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trash")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** [M3.5] SM_TrashBin_Lid. Position it on the back rim in the Blueprint viewport; that pose is "closed". */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trash")
	TObjectPtr<UStaticMeshComponent> HingedLid;

	/** [M3.5] Added to the lid's closed rotation when fully open. If it opens the wrong way, try Pitch -70 or Roll +/-70. */
	UPROPERTY(EditAnywhere, Category = "Trash")
	FRotator LidOpenRotation = FRotator(70.f, 0.f, 0.f);

	/** [M3.5] How fast the lid swings (higher = snappier). Uses REAL time, so it still moves when you're frozen. */
	UPROPERTY(EditAnywhere, Category = "Trash")
	float LidSpeed = 10.f;

private:
	/** True for anything the bin swallows: food, hot sauce, broken pieces. */
	static bool IsBinnable(const AKRItem* Item);
	void PopLidOpen();

	FRotator LidClosedRotation = FRotator::ZeroRotator;
	float LidOpenAmount = 0.f;     // 0 closed, 1 open
	double KeepOpenUntil = 0.0;    // real time
};
