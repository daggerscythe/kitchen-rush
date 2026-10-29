// KRHealthComponent.cpp

#include "Player/KRHealthComponent.h"
#include "Cooking/KRTuningData.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "KitchenRushGameMode.h"

UKRHealthComponent::UKRHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKRHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = UKRTuningData::Get(this)->MaxHealth;
}

UKRHealthComponent* UKRHealthComponent::FindPlayerHealth(const UObject* WorldContext)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(WorldContext, 0);
	return Player ? Player->FindComponentByClass<UKRHealthComponent>() : nullptr;
}

float UKRHealthComponent::GetHealthPercent() const
{
	const float Max = UKRTuningData::Get(this)->MaxHealth;
	return Max > 0.f ? FMath::Clamp(Health / Max, 0.f, 1.f) : 0.f;
}

float UKRHealthComponent::GetDamageFlashAlpha() const
{
	const double Since = GetWorld()->GetRealTimeSeconds() - LastDamageRealTime;
	return FMath::Clamp(1.f - float(Since) / FlashSeconds, 0.f, 1.f);
}

void UKRHealthComponent::ApplyDamage(float Amount)
{
	AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>();
	if (Amount <= 0.f || Health <= 0.f || (GameMode && GameMode->IsGameOver()))
	{
		return;
	}
	Health = FMath::Max(0.f, Health - Amount);
	WorldSecondsSinceDamage = 0.f;
	LastDamageRealTime = GetWorld()->GetRealTimeSeconds();

	if (Health <= 0.f && GameMode)
	{
		GameMode->TriggerGameOver(NSLOCTEXT("KitchenRush", "HealthGameOver", "You're too hurt to keep cooking."));
	}
}

void UKRHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// The player runs at real speed (it cancels out the world slow-down), so DeltaTime here is REAL time.
	// GetWorld()->GetDeltaSeconds() is the WORLD time step, which is what regen should use.
	const float WorldDelta = GetWorld()->GetDeltaSeconds();
	if (Health <= 0.f)
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	WorldSecondsSinceDamage += WorldDelta;
	if (WorldSecondsSinceDamage >= T->RegenDelay && Health < T->MaxHealth)
	{
		Health = FMath::Min(T->MaxHealth, Health + T->RegenPerSecond * WorldDelta);
	}
}
