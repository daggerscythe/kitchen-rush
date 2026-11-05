// KRTimeSubsystem.h
// Dictates the speed of the whole world

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Time/KRTimeSettings.h"
#include "KRTimeSubsystem.generated.h"

UCLASS()
class KITCHENRUSH_API UKRTimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// ---- Subsystem lifetime ----
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Ticking (UTickableWorldSubsystem lets a subsystem tick every frame) ----
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// ---- Things other code calls ----

	/** Call after any action (chop, pick up, place, throw). Time runs at walk speed briefly */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void TriggerActionBurst();

	/** Current world speed: Slow Scale (0.05) standing still, 1.0 walking */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetTimeScale() const { return CurrentScale; }

	/** 0 = walking speed, 1 = fully slowed. Drives the grey "frozen" screen look and the sound */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetFreezeAmount() const;

	/** How many seconds of WORLD time have passed since the level started */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetWorldSecondsElapsed() const { return WorldSeconds; }

	/** True while the player is walking (what makes time run; actions don't count). */
	bool IsWalkingTime() const { return bWalkingTime; }

protected:
	/** Only exist in real game worlds (Play In Editor or a packaged game), never in the editor itself */
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	float CurrentScale = 1.f;      // what the world runs at right now
	float Flow = 0.f;              // 0 = still, 1 = walking; ramps at a fixed speed
	float BurstSecondsLeft = 0.f;  // real seconds left in the current action burst
	float WorldSeconds = 0.f;      // world time that has passed
	bool bWalkingTime = false;
};
