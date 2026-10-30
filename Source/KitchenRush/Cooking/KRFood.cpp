// KRFood.cpp

#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "KitchenRushGameMode.h" // [M4] game events
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AKRFood::AKRFood()
{
	DisplayName = NSLOCTEXT("KitchenRush", "DefaultFoodName", "Food");
}

void AKRFood::SetChopped()
{
	if (bChopped)
	{
		return;
	}
	bChopped = true;
	if (ChoppedMesh)
	{
		Mesh->SetStaticMesh(ChoppedMesh);
	}
}

void AKRFood::AddCookTime(float WorldSeconds)
{
	if (!CanBeCooked())
	{
		return;
	}
	CookSeconds += WorldSeconds;
	if (GetCookStage() != ShownCookStage)
	{
		UpdateCookLook();
	}
}

int32 AKRFood::GetCookStage() const
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	if (CookSeconds < T->SteakCookedAt)     return 0;
	if (CookSeconds <= T->SteakPerfectUntil) return 1;
	if (CookSeconds < T->SteakBurntAt)      return 2;
	return 3;
}

void AKRFood::UpdateCookLook()
{
	ShownCookStage = GetCookStage();
	UMaterialInterface* NewMaterial = nullptr;
	if (ShownCookStage == 1 || ShownCookStage == 2)
	{
		NewMaterial = CookedMaterial;
	}
	else if (ShownCookStage == 3)
	{
		NewMaterial = BurntMaterial;
		AKitchenRushGameMode::NotifyGameEvent(this, EKRGameEvent::SteakBurnt); // [M4]
	}
	if (NewMaterial)
	{
		// Swap just the meat slot; the fat rim keeps its own material.
		Mesh->SetMaterialByName(CookSlotName, NewMaterial);
	}
}

FKRPlatedPart AKRFood::GetPlatedPart() const
{
	FKRPlatedPart Result;
	// [M3.7] One ingredient: its trick shots, a Gus Assist if caught and never dropped, a ground touch if dropped.
	Result.TrickShots = GetTrickShotCount();
	Result.GusAssists = (WasCaughtFromGus() && !HasTouchedGround()) ? 1 : 0;
	Result.GroundTouches = HasTouchedGround() ? 1 : 0;
	switch (FoodType)
	{
	case EKRFoodType::Tomato:  Result.Part = bChopped ? EKRDishPart::ChoppedTomato : EKRDishPart::None; break;
	case EKRFoodType::Lettuce: Result.Part = bChopped ? EKRDishPart::ChoppedLettuce : EKRDishPart::None; break;
	case EKRFoodType::Onion:   Result.Part = bChopped ? EKRDishPart::ChoppedOnion : EKRDishPart::None; break;
	case EKRFoodType::Steak:
	{
		const int32 Stage = GetCookStage();
		Result.bCooked = true;
		Result.Part = (Stage == 1 || Stage == 2) ? EKRDishPart::CookedSteak : EKRDishPart::None;
		Result.bPerfect = (Stage == 1);
		break;
	}
	}
	return Result;
}

FText AKRFood::GetItemName() const
{
	if (CanBeCooked())
	{
		switch (GetCookStage())
		{
		case 0:  return FText::Format(NSLOCTEXT("KitchenRush", "RawFood", "Raw {0}"), DisplayName);
		case 1:  return FText::Format(NSLOCTEXT("KitchenRush", "PerfectFood", "Perfect {0}"), DisplayName);
		case 2:  return FText::Format(NSLOCTEXT("KitchenRush", "WellDoneFood", "Well-done {0}"), DisplayName);
		default: return FText::Format(NSLOCTEXT("KitchenRush", "BurntFood", "Burnt {0}"), DisplayName);
		}
	}
	if (bChopped)
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "ChoppedFood", "Chopped {0}"), DisplayName);
	}
	return DisplayName;
}

FText AKRFood::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (IsInFlight()) // [M3]
	{
		return GetCatchPrompt(HeldItem);
	}
	if (IsHeld() || IsStored())
	{
		return FText::GetEmpty();
	}
	// Holding a plate and looking at loose food: scoop it onto the plate.
	if (const AKRPlate* Plate = Cast<AKRPlate>(HeldItem))
	{
		if (Plate->CanAcceptFood(this))
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "FoodOntoPlate", "Put {0} on {1}"), GetItemName(), Plate->DisplayName);
		}
		return FText::GetEmpty();
	}
	return Super::GetInteractPrompt(HeldItem); // "Pick up ..." with empty hands
}

bool AKRFood::Interact(UKRInteractionComponent* Interactor)
{
	if (IsInFlight()) // [M3]
	{
		return CatchBy(Interactor);
	}
	if (!Interactor || IsHeld() || IsStored())
	{
		return false;
	}
	if (AKRPlate* Plate = Cast<AKRPlate>(Interactor->GetHeldItem()))
	{
		return Plate->AddFood(this);
	}
	return Super::Interact(Interactor);
}
