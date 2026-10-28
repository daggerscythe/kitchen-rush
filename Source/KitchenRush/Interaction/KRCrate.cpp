// KRCrate.cpp

#include "Interaction/KRCrate.h"
#include "Interaction/KRInteractionComponent.h"
#include "Interaction/KRItem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AKRCrate::AKRCrate()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

FText AKRCrate::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (!ItemClass)
	{
		return FText::GetEmpty();
	}
	if (HeldItem)
	{
		return NSLOCTEXT("KitchenRush", "HandsFull", "Hands full");
	}
	const AKRItem* Defaults = ItemClass.GetDefaultObject();
	return FText::Format(NSLOCTEXT("KitchenRush", "TakePrompt", "Take {0}"), Defaults->GetItemName());
}

bool AKRCrate::Interact(UKRInteractionComponent* Interactor)
{
	if (!Interactor || !ItemClass || Interactor->GetHeldItem())
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 60.f);

	AKRItem* NewItem = GetWorld()->SpawnActor<AKRItem>(ItemClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (!NewItem)
	{
		return false;
	}
	if (!Interactor->TryPickUp(NewItem))
	{
		NewItem->Destroy();
		return false;
	}
	return true;
}
