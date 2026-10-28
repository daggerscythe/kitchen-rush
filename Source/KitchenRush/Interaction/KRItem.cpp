// KRItem.cpp

#include "Interaction/KRItem.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"

AKRItem::AKRItem()
{
	// Items don't need to run code every frame
	PrimaryActorTick.bCanEverTick = false;

	// CreateDefaultSubobject makes a component that belongs to this actor
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// "PhysicsActor" is a built-in collision preset for objects that fall and bounce
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetSimulatePhysics(true);
	Mesh->SetUseCCD(true); // continuous collision: stops small fast items passing through counters

	DisplayName = NSLOCTEXT("KitchenRush", "DefaultItemName", "Item");
}

void AKRItem::OnPickedUp(USceneComponent* HoldPoint)
{
	bHeld = true;
	Mesh->SetSimulatePhysics(false); // physics must be off before attaching
	SetActorEnableCollision(false);  // so the held item doesn't block your own line trace
	AttachToComponent(HoldPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AKRItem::OnPlaced(const FVector& Location, const FRotator& Rotation)
{
	bHeld = false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorEnableCollision(true);
	Mesh->SetSimulatePhysics(true);
}

FText AKRItem::GetInteractPrompt(const AKRItem* HeldItem) const
{
	// Can't pick this up if it's already in someone's hands, or if your hands are full
	if (bHeld || HeldItem)
	{
		return FText::GetEmpty();
	}
	return FText::Format(NSLOCTEXT("KitchenRush", "PickUpPrompt", "Pick up {0}"), DisplayName);
}

bool AKRItem::Interact(UKRInteractionComponent* Interactor)
{
	return Interactor && Interactor->TryPickUp(this);
}
