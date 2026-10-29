// KRShards.h
// [M3.5] What's left of a dropped plate or bowl: a small pile of sharp pieces on the floor.
// Stepping on the pile hurts (once each time you step onto it). Stand next to it and press E
// to pick the whole pile up, then bin it (E at the trash, or throw it in).
// Make BP_Shards from this and pick the four shard meshes.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/KRItem.h"
#include "KRShards.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class KITCHENRUSH_API AKRShards : public AKRItem
{
	GENERATED_BODY()

public:
	AKRShards();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	// The root Mesh (from AKRItem) is the first shard and does the physics. These three just sit around it.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shards")
	TObjectPtr<UStaticMeshComponent> Shard2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shards")
	TObjectPtr<UStaticMeshComponent> Shard3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shards")
	TObjectPtr<UStaticMeshComponent> Shard4;

	/** How far (cm) the pieces scatter around the first one on the floor. */
	UPROPERTY(EditAnywhere, Category = "Shards")
	float SpreadRadius = 22.f;

	/** [M3.6] An invisible ball around the whole pile that only the look-at trace sees,
	 *  so aiming anywhere near the pile picks it up (not just at one tiny shard). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shards")
	TObjectPtr<USphereComponent> PickArea;

private:
	void LayOut(float Spread);

	/** [M3.6] Once the pile stops moving: physics off, lie flat, sit exactly on the floor (no pieces poking through). */
	void SettleOnFloor();

	TArray<FVector2D> Offsets;  // where each extra piece sits, as a fraction of the spread
	bool bPlayerOnPile = false; // so one step = one hit, not damage every frame
	bool bShownHeldLayout = false;
	bool bSettled = false;          // [M3.6]
	float RestingSeconds = 0.f;     // [M3.6] how long it's been nearly still
};
