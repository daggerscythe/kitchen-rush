// KRHealthComponent.h
// The player's health. Hurt by knives, hot sauce, fire and spills. Comes back while WORLD time passes

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KRHealthComponent.generated.h"

UCLASS(ClassGroup = (KitchenRush), meta = (BlueprintSpawnableComponent))
class KITCHENRUSH_API UKRHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKRHealthComponent();

	/** Take damage. Ignored after Game Over. */
	void ApplyDamage(float Amount);

	/** Finds the player's health component (or nullptr). Handy for hazards. */
	static UKRHealthComponent* FindPlayerHealth(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	/** 1 right after being hurt, fading to 0 over FlashSeconds of REAL time (so it fades while frozen). */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetDamageFlashAlpha() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	/** How long the red flash takes to fade, in real seconds. */
	UPROPERTY(EditAnywhere, Category = "Health")
	float FlashSeconds = 0.5f;

private:
	float Health = 100.f;
	float WorldSecondsSinceDamage = 999.f;
	double LastDamageRealTime = -100.0;
};
