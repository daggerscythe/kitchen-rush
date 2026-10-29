// KRHazardZone.h
// A patch of danger that hurts the player: grease fire, spill when the soup boils over

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KRHazardZone.generated.h"

class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRHazardZone : public AActor
{
	GENERATED_BODY()

public:
	AKRHazardZone();

	/** Call right after spawning. All numbers are world seconds / cm. */
	void Start(float InSeconds, float InDamagePerSecond, float InRadius, bool bInBlocksBurner);

	/** True if a burner-blocking zone (a grease fire) is burning on top of Location. */
	static bool IsBurnerBlocked(UWorld* World, const FVector& Location);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	/** What the hazard looks like (SM_Flame, SM_SpillPuddle). No collision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard")
	TObjectPtr<UStaticMeshComponent> Visual;

	/** 0 = still. 0.15 makes a flame flicker in height (it freezes with time, which looks great). */
	UPROPERTY(EditAnywhere, Category = "Hazard", meta = (ClampMin = "0", ClampMax = "1"))
	float FlickerAmount = 0.f;

	/** A player this far above or below the zone is safe (cm). */
	UPROPERTY(EditAnywhere, Category = "Hazard")
	float HeightRange = 150.f;

private:
	float SecondsLeft = 10.f;
	float DamagePerSecond = 10.f;
	float Radius = 100.f;
	bool bBlocksBurner = false;
	float Age = 0.f;
	FVector BaseScale = FVector::OneVector;
};
