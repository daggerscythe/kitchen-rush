// KRHazardZone.cpp

#include "Hazards/KRHazardZone.h"
#include "Player/KRHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AKRHazardZone::AKRHazardZone()
{
	PrimaryActorTick.bCanEverTick = true;

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	SetRootComponent(Visual);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetCastShadow(false);
}

void AKRHazardZone::BeginPlay()
{
	Super::BeginPlay();
	BaseScale = Visual->GetRelativeScale3D();
}

void AKRHazardZone::Start(float InSeconds, float InDamagePerSecond, float InRadius, bool bInBlocksBurner)
{
	SecondsLeft = InSeconds;
	DamagePerSecond = InDamagePerSecond;
	Radius = InRadius;
	bBlocksBurner = bInBlocksBurner;
}

bool AKRHazardZone::IsBurnerBlocked(UWorld* World, const FVector& Location)
{
	if (!World)
	{
		return false;
	}
	for (TActorIterator<AKRHazardZone> It(World); It; ++It)
	{
		if (It->bBlocksBurner && FVector::Dist2D(It->GetActorLocation(), Location) < 30.f)
		{
			return true;
		}
	}
	return false;
}

void AKRHazardZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); // DeltaSeconds is WORLD time: when you freeze, the fire stops hurting too

	Age += DeltaSeconds;
	SecondsLeft -= DeltaSeconds;
	if (SecondsLeft <= 0.f)
	{
		Destroy();
		return;
	}

	if (FlickerAmount > 0.f)
	{
		const float Flicker = 1.f + FlickerAmount * FMath::Sin(Age * 14.f) * FMath::Sin(Age * 5.3f);
		Visual->SetRelativeScale3D(FVector(BaseScale.X, BaseScale.Y, BaseScale.Z * Flicker));
	}

	// Hurt the player if they're standing in it.
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector Delta = Player->GetActorLocation() - GetActorLocation();
		if (Delta.Size2D() <= Radius && FMath::Abs(Delta.Z) <= HeightRange)
		{
			if (UKRHealthComponent* Health = Player->FindComponentByClass<UKRHealthComponent>())
			{
				Health->ApplyDamage(DamagePerSecond * DeltaSeconds);
			}
		}
	}
}
