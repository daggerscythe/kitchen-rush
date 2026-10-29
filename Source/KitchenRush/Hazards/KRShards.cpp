// KRShards.cpp

#include "Hazards/KRShards.h"
#include "Cooking/KRTuningData.h"
#include "Player/KRHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "KitchenRushGameMode.h"

static UStaticMeshComponent* MakePiece(AKRShards* Owner, UStaticMeshComponent* Parent, const TCHAR* Name)
{
	UStaticMeshComponent* Piece = Owner->CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Piece->SetupAttachment(Parent);
	Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision); // only the root piece collides
	return Piece;
}

AKRShards::AKRShards()
{
	PrimaryActorTick.bCanEverTick = true; // watches for the player stepping on it
	DisplayName = NSLOCTEXT("KitchenRush", "ShardsName", "Broken Plate");

	Shard2 = MakePiece(this, Mesh, TEXT("Shard 2"));
	Shard3 = MakePiece(this, Mesh, TEXT("Shard 3"));
	Shard4 = MakePiece(this, Mesh, TEXT("Shard 4"));

	// [M3.6] Pick-up area: blocks ONLY the Visibility channel (what the look-at trace uses), so it's
	// easy to aim at but nothing physical bumps into it.
	PickArea = CreateDefaultSubobject<USphereComponent>(TEXT("Pick Area"));
	PickArea->SetupAttachment(Mesh);
	PickArea->InitSphereRadius(30.f);
	PickArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickArea->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PickArea->SetGenerateOverlapEvents(false);
	PickArea->SetRelativeLocation(FVector(0.f, 0.f, 8.f));
}

// [M3.6]
void AKRShards::SettleOnFloor()
{
	bSettled = true;
	Mesh->SetSimulatePhysics(false);

	// Lie flat (keep only the turn), then find the floor straight below and sit on it.
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KRShardsFloor), false, this);
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 20.f);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start - FVector(0.f, 0.f, 80.f), ECC_WorldStatic, Params))
	{
		SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, 0.5f));
	}
}

void AKRShards::BeginPlay()
{
	Super::BeginPlay();
	// Scatter the three extra pieces around the first one, each at a random angle.
	for (int32 i = 0; i < 3; ++i)
	{
		const float Angle = FMath::DegreesToRadians(120.f * i + FMath::FRandRange(-30.f, 30.f));
		Offsets.Add(FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * FMath::FRandRange(0.6f, 1.f));
	}
	Shard2->SetRelativeRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));
	Shard3->SetRelativeRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));
	Shard4->SetRelativeRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));
	LayOut(SpreadRadius);
}

void AKRShards::LayOut(float Spread)
{
	UStaticMeshComponent* Pieces[] = { Shard2, Shard3, Shard4 };
	for (int32 i = 0; i < 3 && i < Offsets.Num(); ++i)
	{
		Pieces[i]->SetRelativeLocation(FVector(Offsets[i].X * Spread, Offsets[i].Y * Spread, 0.f));
	}
}

void AKRShards::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// In your hands the pile gathers up small; back on the floor it spreads out again.
	const bool bGathered = IsHeld() || IsStored();
	if (bGathered != bShownHeldLayout)
	{
		bShownHeldLayout = bGathered;
		LayOut(bGathered ? SpreadRadius * 0.25f : SpreadRadius);
	}
	if (bGathered)
	{
		bPlayerOnPile = false;
		bSettled = false; // it'll settle again wherever it's put down
		RestingSeconds = 0.f;
		return;
	}

	// [M3.6] Settle: physics tilts the pile and the loose pieces poke through the floor, so once it has
	// been nearly still for a moment (world time), switch physics off and lay it flat on the floor.
	if (!bSettled && Mesh->IsSimulatingPhysics())
	{
		RestingSeconds = (GetVelocity().Size() < 5.f) ? RestingSeconds + DeltaSeconds : 0.f;
		if (RestingSeconds > 0.2f)
		{
			SettleOnFloor();
		}
	}

	// Is the player standing on the pile? Compare their feet to the pile, sideways and in height.
	const ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	const FVector Feet = Player->GetActorLocation() - FVector(0.f, 0.f, Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	const bool bOnPile = FVector::Dist2D(Feet, GetActorLocation()) <= T->ShardRadius && FMath::Abs(Feet.Z - GetActorLocation().Z) <= 30.f;

	if (bOnPile && !bPlayerOnPile)
	{
		if (UKRHealthComponent* Health = Player->FindComponentByClass<UKRHealthComponent>())
		{
			Health->ApplyDamage(T->ShardStepDamage);
		}
		if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
		{
			GameMode->ShowMessage(NSLOCTEXT("KitchenRush", "SteppedOnShards", "Ouch! Broken pieces underfoot. Pick them up (E) and bin them."));
		}
	}
	bPlayerOnPile = bOnPile;
}
