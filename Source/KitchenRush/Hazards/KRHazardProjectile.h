// KRHazardProjectile.h
// Dangerous things Gus sends flying.
//  - Knife: flies almost straight (low gravity), 20 damage if it hits you, sticks in whatever else it hits.
//    It can't be caught: trying costs 10 health. Letting one pass close by earns the dodge bonus.
//  - Hot sauce: a normal throwable bottle. It shatters on whatever it hits (10 damage if you're within 1.5 m),
//    but it's safe to catch. Once caught you can carry it, bin it, or throw it yourself.
// Make BP_Knife and BP_HotSauce from this class and pick the Kind and mesh.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/KRItem.h"
#include "KRHazardProjectile.generated.h"

UENUM(BlueprintType)
enum class EKRHazardKind : uint8
{
	Knife,
	HotSauce
};

UCLASS()
class KITCHENRUSH_API AKRHazardProjectile : public AKRItem
{
	GENERATED_BODY()

public:
	AKRHazardProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
	EKRHazardKind Kind = EKRHazardKind::Knife;

	/** Knife only: turn the mesh so the blade points where it flies. If knives fly sideways or
	 *  handle-first, change Yaw here (try 90, -90 or 180). */
	UPROPERTY(EditAnywhere, Category = "Hazard")
	FRotator BladeRotationOffset = FRotator::ZeroRotator;

	bool IsKnife() const { return Kind == EKRHazardKind::Knife; }

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void PostThrown(const FVector& Velocity) override;
	virtual void HandleFlightHit(AActor* OtherActor, const FHitResult& Hit) override;

private:
	void KnifeHit(AActor* OtherActor, const FHitResult& Hit);
	void Shatter();
	void Report(const FText& Text) const;

	float ClosestToPlayer = 1e9f; // knives: closest the knife got to the player this flight (cm)
};
