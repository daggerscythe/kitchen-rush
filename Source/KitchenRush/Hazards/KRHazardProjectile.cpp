// KRHazardProjectile.cpp

#include "Hazards/KRHazardProjectile.h"
#include "Cooking/KRTuningData.h"
#include "Player/KRHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "KitchenRushGameMode.h"

AKRHazardProjectile::AKRHazardProjectile()
{
	PrimaryActorTick.bCanEverTick = true; // knives steer their own gravity and watch for near misses
	DisplayName = NSLOCTEXT("KitchenRush", "KnifeName", "Knife");
}

void AKRHazardProjectile::Report(const FText& Text) const
{
	if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
	{
		GameMode->ShowMessage(Text);
	}
}

FText AKRHazardProjectile::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (IsKnife())
	{
		// A flying knife shows a prompt as a warning; a stuck one shows nothing.
		return IsInFlight() ? NSLOCTEXT("KitchenRush", "KnifePrompt", "Catch Knife? (sharp!)") : FText::GetEmpty();
	}
	return Super::GetInteractPrompt(HeldItem); // hot sauce behaves like any item: catch / pick up
}

bool AKRHazardProjectile::Interact(UKRInteractionComponent* Interactor)
{
	if (!IsKnife())
	{
		return Super::Interact(Interactor);
	}
	if (!IsInFlight())
	{
		return false;
	}
	// Grabbing a flying knife by the blade: ouch. It keeps flying.
	if (UKRHealthComponent* Health = UKRHealthComponent::FindPlayerHealth(this))
	{
		Health->ApplyDamage(UKRTuningData::Get(this)->KnifeCatchDamage);
	}
	Report(NSLOCTEXT("KitchenRush", "KnifeCatch", "Ouch! Knives can't be caught. Dodge them!"));
	return true;
}

void AKRHazardProjectile::PostThrown(const FVector& Velocity)
{
	ClosestToPlayer = 1e9f;
	if (!IsKnife())
	{
		return;
	}
	// Knives fly point-first and don't tumble, so you can read where they're going.
	const FQuat Facing = Velocity.ToOrientationQuat() * BladeRotationOffset.Quaternion();
	SetActorRotation(Facing, ETeleportType::TeleportPhysics);
	Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	Mesh->SetEnableGravity(false); // we add our own, weaker gravity in Tick
	Mesh->SetPhysicsLinearVelocity(Velocity);
	SetLifeSpan(15.f); // world seconds: a knife that misses everything doesn't hang around forever
}

void AKRHazardProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); // world time

	if (!IsKnife() || !IsInFlight())
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);

	// Weak gravity. bAccelChange = true means "this is an acceleration", so mass doesn't matter.
	Mesh->AddForce(FVector(0.f, 0.f, GetWorld()->GetGravityZ() * T->KnifeGravityScale), NAME_None, true);

	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		ClosestToPlayer = FMath::Min(ClosestToPlayer, FVector::Dist(Player->GetActorLocation(), GetActorLocation()));
	}
}

void AKRHazardProjectile::HandleFlightHit(AActor* OtherActor, const FHitResult& Hit)
{
	if (IsKnife())
	{
		KnifeHit(OtherActor, Hit);
		return;
	}

	// Hot sauce: a throw into the trash is safe; anything else shatters it.
	EndFlight();
	if (TryLandIn(OtherActor))
	{
		return;
	}
	Shatter();
}

void AKRHazardProjectile::KnifeHit(AActor* OtherActor, const FHitResult& Hit)
{
	EndFlight();
	const UKRTuningData* T = UKRTuningData::Get(this);
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);

	if (OtherActor && OtherActor == Player)
	{
		if (UKRHealthComponent* Health = Player->FindComponentByClass<UKRHealthComponent>())
		{
			Health->ApplyDamage(T->KnifeHitDamage);
		}
		AKitchenRushGameMode::NotifyGameEvent(this, EKRGameEvent::KnifeHit); // [M4]
		// Clatter to the floor and vanish.
		Mesh->SetEnableGravity(true);
		SetLifeSpan(3.f);
		return;
	}

	// Stuck in a wall, counter or crate: freeze in place for a bit.
	Mesh->SetSimulatePhysics(false);
	SetActorEnableCollision(false);
	SetLifeSpan(6.f);

	// A near miss you dodged earns a bonus.
	if (ClosestToPlayer <= T->DodgeDistance)
	{
		if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
		{
			GameMode->AddBonus(FMath::RoundToInt(T->DodgeBonus), NSLOCTEXT("KitchenRush", "Dodged", "Dodged!"));
		}
	}
}

void AKRHazardProjectile::Shatter()
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (FVector::Dist(Player->GetActorLocation(), GetActorLocation()) <= T->SauceShatterRadius)
		{
			if (UKRHealthComponent* Health = Player->FindComponentByClass<UKRHealthComponent>())
			{
				Health->ApplyDamage(T->SauceShatterDamage);
			}
			Report(NSLOCTEXT("KitchenRush", "SauceHit", "Hot sauce in the face! Catch the bottles next time."));
		}
	}
	Destroy();
}
