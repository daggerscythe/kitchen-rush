// KRStove.cpp

#include "Cooking/KRStove.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/CollisionProfile.h"
#include "Hazards/KRHazardZone.h"

static void SetupBurnerMarker(USphereComponent* Burner)
{
	Burner->InitSphereRadius(12.f);
	Burner->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Burner->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Burner->SetGenerateOverlapEvents(false);
	Burner->CanCharacterStepUpOn = ECB_No;
	Burner->ShapeColor = FColor::Orange;
	Burner->SetHiddenInGame(true);
}

AKRStove::AKRStove()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	BurnerA = CreateDefaultSubobject<USphereComponent>(TEXT("Burner A"));
	BurnerA->SetupAttachment(Mesh);
	BurnerA->SetRelativeLocation(FVector(27.f, 30.f, 92.f));
	SetupBurnerMarker(BurnerA);

	BurnerB = CreateDefaultSubobject<USphereComponent>(TEXT("Burner B"));
	BurnerB->SetupAttachment(Mesh);
	BurnerB->SetRelativeLocation(FVector(73.f, 30.f, 92.f));
	SetupBurnerMarker(BurnerB);
}

bool AKRStove::IsNearBurner(UWorld* World, const FVector& Location, float Radius, float Height)
{
	if (!World)
	{
		return false;
	}
	// TActorIterator walks every actor of a class in the level.
	for (TActorIterator<AKRStove> It(World); It; ++It)
	{
		for (const USphereComponent* Burner : { It->BurnerA.Get(), It->BurnerB.Get() })
		{
			if (!Burner || AKRHazardZone::IsBurnerBlocked(World, Burner->GetComponentLocation())) // fire = burner off
			{
				continue;
			}
			const FVector Delta = Location - Burner->GetComponentLocation();
			if (Delta.Size2D() <= Radius && FMath::Abs(Delta.Z) <= Height)
			{
				return true;
			}
		}
	}
	return false;
}
