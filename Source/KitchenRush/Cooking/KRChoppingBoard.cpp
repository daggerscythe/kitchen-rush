// KRChoppingBoard.cpp 

#include "Cooking/KRChoppingBoard.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"

AKRChoppingBoard::AKRChoppingBoard()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	FoodSlot = CreateDefaultSubobject<USceneComponent>(TEXT("Food Slot"));
	FoodSlot->SetupAttachment(Mesh);
	FoodSlot->SetRelativeLocation(FVector(0.f, 0.f, 3.f));
}

FText AKRChoppingBoard::GetInteractPrompt(const AKRItem* HeldItem) const
{
	const int32 ChopsNeeded = UKRTuningData::Get(this)->ChopsNeeded;

	if (!Food)
	{
		// Empty board: offer to put choppable food on it.
		const AKRFood* HeldFood = Cast<AKRFood>(HeldItem);
		if (HeldFood && HeldFood->CanBeChopped())
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "PutOnBoard", "Put {0} on board"), HeldFood->GetItemName());
		}
		return FText::GetEmpty();
	}

	if (!Food->IsChopped())
	{
		if (HeldItem)
		{
			return NSLOCTEXT("KitchenRush", "ChopNeedsHands", "Hands full (can't chop)");
		}
		return FText::Format(NSLOCTEXT("KitchenRush", "ChopPrompt", "Chop ({0}/{1})"), Chops, ChopsNeeded);
	}

	// Chopped food on the board.
	if (!HeldItem)
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "TakeFromBoard", "Take {0}"), Food->GetItemName());
	}
	if (const AKRPlate* Plate = Cast<AKRPlate>(HeldItem))
	{
		if (Plate->CanAcceptFood(Food))
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "BoardToPlate", "Put {0} on {1}"), Food->GetItemName(), Plate->DisplayName);
		}
	}
	return FText::GetEmpty();
}

bool AKRChoppingBoard::Interact(UKRInteractionComponent* Interactor)
{
	if (!Interactor)
	{
		return false;
	}
	AKRItem* Held = Interactor->GetHeldItem();

	if (!Food)
	{
		AKRFood* HeldFood = Cast<AKRFood>(Held);
		if (!HeldFood || !HeldFood->CanBeChopped())
		{
			return false;
		}
		Interactor->ReleaseHeldItem();
		HeldFood->OnStored(FoodSlot, FVector::ZeroVector);
		Food = HeldFood;
		Chops = 0;
		return true;
	}

	if (!Food->IsChopped())
	{
		if (Held)
		{
			return false;
		}
		++Chops;
		if (Chops >= UKRTuningData::Get(this)->ChopsNeeded)
		{
			Food->SetChopped();
		}
		return true;
	}

	// Chopped: hand it to the player or onto their plate.
	AKRFood* Chopped = Food;
	if (!Held)
	{
		Food = nullptr;
		return Interactor->TryPickUp(Chopped);
	}
	if (AKRPlate* Plate = Cast<AKRPlate>(Held))
	{
		if (Plate->CanAcceptFood(Chopped))
		{
			Food = nullptr;
			return Plate->AddFood(Chopped);
		}
	}
	return false;
}
