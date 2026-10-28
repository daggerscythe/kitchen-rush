// KRItem.cpp

#include "Interaction/KRItem.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"

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

	DisplayName = NSLOCTEXT("KitchenRush", "DefaultItemName", "Item");
}

UStaticMeshComponent* AKRItem::GetMesh() const
{
	return Mesh;
}

void AKRItem::OnPickedUp(USceneComponent* HoldPoint)
{
	bHeld = true;
	bStored = false;
	Mesh->SetSimulatePhysics(false); // physics must be off before attaching
	SetActorEnableCollision(false);  // so the held item doesn't block your own line trace
	AttachToComponent(HoldPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AKRItem::OnPlaced(const FVector& Location, const FRotator& Rotation)
{
	bHeld = false;
	bStored = false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorEnableCollision(true);
	Mesh->SetSimulatePhysics(true);
}

void AKRItem::OnStored(USceneComponent* Parent, const FVector& RelativeLocation)
{
	bHeld = false;
	bStored = true;
	Mesh->SetSimulatePhysics(false);
	SetActorEnableCollision(false); // the container gets the line trace, not the item inside it
	AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
}

FText AKRItem::GetInteractPrompt(const AKRItem* HeldItem) const
{
	// Can't pick up if in someone's hands / container, if hands are full, or if item can't be carried.
	if (bHeld || bStored || HeldItem || !bCanBePickedUp)
	{
		return FText::GetEmpty();
	}
	return FText::Format(NSLOCTEXT("KitchenRush", "PickUpPrompt", "Pick up {0}"), GetItemName());
}

bool AKRItem::Interact(UKRInteractionComponent* Interactor)
{
	if (!bCanBePickedUp || bStored)
	{
		return false;
	}
	return Interactor && Interactor->TryPickUp(this);
}
