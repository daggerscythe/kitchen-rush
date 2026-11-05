// KRItem.cpp

#include "Interaction/KRItem.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "KitchenRushGameMode.h" // game events
#include "Audio/KRAudioSubsystem.h"

AKRItem::AKRItem()
{
	// Items don't need to run code every frame (cookware turns this back on).
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// "PhysicsActor" is a built-in collision preset for objects that fall and bounce.
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetSimulatePhysics(true);
	Mesh->SetUseCCD(true); // continuous collision: stops small fast items passing through counters
	Mesh->SetNotifyRigidBodyCollision(true); // "Simulation Generates Hit Events": needed for OnMeshHit
	Mesh->SetGenerateOverlapEvents(true);    // so the pass can notice a plate flying through it

	DisplayName = NSLOCTEXT("KitchenRush", "DefaultItemName", "Item");
}

void AKRItem::BeginPlay()
{
	Super::BeginPlay();
	// AddDynamic hooks our function to the mesh's hit event. The function must be a UFUNCTION.
	Mesh->OnComponentHit.AddDynamic(this, &AKRItem::OnMeshHit);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

UStaticMeshComponent* AKRItem::GetMesh() const
{
	return Mesh;
}

void AKRItem::OnPickedUp(USceneComponent* HoldPoint)
{
	bHeld = true;
	bStored = false;
	EndFlight();
	Mesh->SetSimulatePhysics(false); // physics must be off before attaching
	SetActorEnableCollision(false);  // so the held item doesn't block your own line trace
	AttachToComponent(HoldPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AKRItem::OnPlaced(const FVector& Location, const FRotator& Rotation)
{
	bHeld = false;
	bStored = false;
	EndFlight();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorEnableCollision(true);
	Mesh->SetSimulatePhysics(true);
}

void AKRItem::OnStored(USceneComponent* Parent, const FVector& RelativeLocation)
{
	bHeld = false;
	bStored = true;
	EndFlight();
	Mesh->SetSimulatePhysics(false);
	SetActorEnableCollision(false); // the container gets the line trace, not the item inside it
	AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
}

void AKRItem::OnThrown(const FVector& StartLocation, const FVector& Velocity, bool bFromGus)
{
	bHeld = false;
	bStored = false;
	bInFlight = true;
	bLaunchedByGus = bFromGus;
	// (bWasCaughtFromGus is NOT reset: re-throwing a caught ingredient into the pan keeps its bonus.)

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorEnableCollision(true);
	// Gus's throws can hit you; your own throws (and anything lying around) pass through you.
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, bFromGus ? ECR_Block : ECR_Ignore);
	Mesh->SetSimulatePhysics(true);

	// Physics runs on world time, so when you're frozen the item hangs in the air: Superhot style.
	Mesh->SetPhysicsLinearVelocity(Velocity);
	Mesh->SetPhysicsAngularVelocityInDegrees(FVector(FMath::FRandRange(-180.f, 180.f), FMath::FRandRange(-180.f, 180.f), 0.f));

	PostThrown(Velocity);
}

void AKRItem::EndFlight()
{
	bInFlight = false;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // PhysicsActor's normal setting
}

FText AKRItem::GetCatchPrompt(const AKRItem* HeldItem) const
{
	if (!HeldItem)
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "CatchPrompt", "Catch {0}"), GetItemName());
	}
	// AKRItem is itself an IKRInteractable, so we can ask the held item directly.
	if (HeldItem->CanReceiveItem(this))
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "CatchWithPrompt", "Catch {0} with {1}"), GetItemName(), HeldItem->DisplayName);
	}
	return FText::GetEmpty();
}

bool AKRItem::CatchBy(UKRInteractionComponent* Interactor)
{
	if (!Interactor || !bInFlight)
	{
		return false;
	}
	const bool bFromGus = bLaunchedByGus;
	bool bCaught = false;
	if (AKRItem* Held = Interactor->GetHeldItem())
	{
		bCaught = Held->CanReceiveItem(this) && Held->ReceiveThrownItem(this); // e.g. straight onto a plate
	}
	else
	{
		bCaught = Interactor->TryPickUp(this);
	}
	if (bCaught && bFromGus)
	{
		bWasCaughtFromGus = true;
	}
	if (bCaught)
	{
		AKitchenRushGameMode::NotifyGameEvent(this, EKRGameEvent::Catch, NAME_None, bFromGus ? 1.f : 0.f);
	}
	return bCaught;
}

void AKRItem::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == this)
	{
		return;
	}
	if (!bTouchedGround && IsFloorHit(Hit))
	{
		bTouchedGround = true;
	}
	OnPhysicsHit(OtherActor, Hit);
	if (!bInFlight || IsActorBeingDestroyed()) // (a plate may have just shattered)
	{
		return;
	}
	// Two things flying together (a Gus burst) bump each other, but that shouldn't end either flight.
	if (const AKRItem* OtherItem = Cast<AKRItem>(OtherActor))
	{
		if (OtherItem->IsInFlight())
		{
			return;
		}
	}
	HandleFlightHit(OtherActor, Hit);
}

// (moved here from AKRPlate so every item can use it)
bool AKRItem::IsFloorHit(const FHitResult& Hit) const
{
	const ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player || Hit.ImpactNormal.Z < 0.7f)
	{
		return false;
	}
	const float FeetZ = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return Hit.ImpactPoint.Z <= FeetZ + 25.f;
}

bool AKRItem::TryLandIn(AActor* OtherActor)
{
	IKRInteractable* Target = Cast<IKRInteractable>(OtherActor);
	if (!Target || !Target->CanReceiveItem(this))
	{
		return false;
	}
	// Mark this place BEFORE ReceiveThrownItem: the pot reads the marks, then destroys the tomato.
	// A bit per place, so throwing the same tomato onto the board ten times still counts once.
	const uint8 OldMask = TrickShotMask;
	const uint8 Place = bLaunchedByGus ? 0 : Target->GetTrickShotFlag();
	TrickShotMask |= Place;
	if (Target->ReceiveThrownItem(this))
	{
		if (Place != 0)
		{
			AKitchenRushGameMode::NotifyGameEvent(this, EKRGameEvent::TrickShot, NAME_None, 0.f, Place);
		}
		return true;
	}
	TrickShotMask = OldMask;
	return false;
}

void AKRItem::HandleFlightHit(AActor* OtherActor, const FHitResult& Hit)
{
	EndFlight(); // the first thing it touches ends the flight
	if (TryLandIn(OtherActor))
	{
		return; // landed in a pan / pot / on a plate. Don't touch "this" after this point.
	}
	UKRAudioSubsystem::PlayAt(this, GetImpactSound(), Hit.ImpactPoint); // thud / clang
	if (IKRInteractable* Target = Cast<IKRInteractable>(OtherActor))
	{
		Target->OnHitByThrownItem(this); // e.g. knock the pot off the burner
	}
}

FText AKRItem::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (bInFlight)
	{
		return GetCatchPrompt(HeldItem);
	}
	// Can't pick up if in someone's hands / container, if hands are full, or if item can't be carried.
	if (bHeld || bStored || HeldItem || !bCanBePickedUp)
	{
		return FText::GetEmpty();
	}
	return FText::Format(NSLOCTEXT("KitchenRush", "PickUpPrompt", "Pick up {0}"), GetItemName());
}

bool AKRItem::Interact(UKRInteractionComponent* Interactor)
{
	if (bInFlight)
	{
		return CatchBy(Interactor);
	}
	if (!bCanBePickedUp || bStored)
	{
		return false;
	}
	return Interactor && Interactor->TryPickUp(this);
}
