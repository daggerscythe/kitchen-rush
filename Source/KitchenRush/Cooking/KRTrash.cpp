// KRTrash.cpp

#include "Cooking/KRTrash.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"

AKRTrash::AKRTrash()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

FText AKRTrash::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (const AKRPlate* Plate = Cast<AKRPlate>(HeldItem))
	{
		if (!Plate->IsEmpty())
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "EmptyPlatePrompt", "Empty {0}"), Plate->DisplayName);
		}
		return FText::GetEmpty();
	}
	if (const AKRFood* Food = Cast<AKRFood>(HeldItem))
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "TrashFood", "Throw away {0}"), Food->GetItemName());
	}
	return FText::GetEmpty();
}

bool AKRTrash::Interact(UKRInteractionComponent* Interactor)
{
	if (!Interactor)
	{
		return false;
	}
	AKRItem* Held = Interactor->GetHeldItem();
	if (AKRPlate* Plate = Cast<AKRPlate>(Held))
	{
		if (Plate->IsEmpty())
		{
			return false;
		}
		Plate->ClearContents(); // the plate stays in your hands
		return true;
	}
	if (Cast<AKRFood>(Held))
	{
		Interactor->ReleaseHeldItem()->Destroy();
		return true;
	}
	return false;
}
