// KRPlate.cpp

#include "Cooking/KRPlate.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRTuningData.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"

AKRPlate::AKRPlate()
{
	DisplayName = NSLOCTEXT("KitchenRush", "PlateName", "Plate");

	SoupFill = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Soup Fill"));
	SoupFill->SetupAttachment(Mesh);
	SoupFill->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SoupFill->SetRelativeLocation(FVector(0.f, 0.f, 5.f));
	SoupFill->SetVisibility(false);
}

bool AKRPlate::CanAcceptFood(const AKRFood* Food) const
{
	return Food && !bIsBowl && Foods.Num() < UKRTuningData::Get(this)->MaxPlateItems;
}

bool AKRPlate::AddFood(AKRFood* Food)
{
	if (!CanAcceptFood(Food))
	{
		return false;
	}
	// Arrange up to 3 items in a small triangle on the plate.
	const int32 Index = Foods.Num();
	const float Angle = FMath::DegreesToRadians(120.f * Index + 90.f);
	const FVector Offset(FMath::Cos(Angle) * 5.f, FMath::Sin(Angle) * 5.f, 1.5f);

	Food->OnStored(Mesh, Offset);
	Foods.Add(Food);
	return true;
}

bool AKRPlate::CanAcceptSoup() const
{
	return bIsBowl && !bHasSoup;
}

void AKRPlate::AddSoup(bool bPerfect)
{
	bHasSoup = true;
	bSoupPerfect = bPerfect;
	SoupFill->SetVisibility(true);
}

void AKRPlate::ClearContents()
{
	for (AKRFood* Food : Foods)
	{
		if (Food)
		{
			Food->Destroy();
		}
	}
	Foods.Empty();
	bHasSoup = false;
	bSoupPerfect = false;
	SoupFill->SetVisibility(false);
}

bool AKRPlate::IsEmpty() const
{
	return Foods.Num() == 0 && !bHasSoup;
}

TArray<FKRPlatedPart> AKRPlate::GetPlatedParts() const
{
	TArray<FKRPlatedPart> Parts;
	for (const AKRFood* Food : Foods)
	{
		if (Food)
		{
			Parts.Add(Food->GetPlatedPart());
		}
	}
	if (bHasSoup)
	{
		FKRPlatedPart Soup;
		Soup.Part = EKRDishPart::TomatoSoup;
		Soup.bCooked = true;
		Soup.bPerfect = bSoupPerfect;
		Parts.Add(Soup);
	}
	return Parts;
}

FText AKRPlate::GetContentsText() const
{
	TArray<FString> Names;
	for (const AKRFood* Food : Foods)
	{
		if (Food)
		{
			Names.Add(Food->GetItemName().ToString());
		}
	}
	if (bHasSoup)
	{
		Names.Add(bSoupPerfect ? TEXT("Perfect Tomato Soup") : TEXT("Tomato Soup"));
	}
	if (Names.Num() == 0)
	{
		return NSLOCTEXT("KitchenRush", "EmptyPlate", "empty");
	}
	return FText::FromString(FString::Join(Names, TEXT(", ")));
}

FText AKRPlate::GetItemName() const
{
	if (IsEmpty())
	{
		return DisplayName;
	}
	return FText::Format(NSLOCTEXT("KitchenRush", "PlateWithItems", "{0} ({1})"), DisplayName, GetContentsText());
}

FText AKRPlate::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (IsHeld() || IsStored())
	{
		return FText::GetEmpty();
	}
	// Holding food and looking at a plate on the counter: put the food on it.
	if (const AKRFood* Food = Cast<AKRFood>(HeldItem))
	{
		if (CanAcceptFood(Food))
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "AddToPlate", "Put {0} on {1}"), Food->GetItemName(), DisplayName);
		}
		return FText::GetEmpty();
	}
	return Super::GetInteractPrompt(HeldItem);
}

bool AKRPlate::Interact(UKRInteractionComponent* Interactor)
{
	if (!Interactor || IsHeld() || IsStored())
	{
		return false;
	}
	if (AKRFood* Food = Cast<AKRFood>(Interactor->GetHeldItem()))
	{
		if (!CanAcceptFood(Food))
		{
			return false;
		}
		Interactor->ReleaseHeldItem();
		return AddFood(Food);
	}
	return Super::Interact(Interactor);
}

void AKRPlate::Destroyed()
{
	for (AKRFood* Food : Foods)
	{
		if (Food)
		{
			Food->Destroy();
		}
	}
	Foods.Empty();
	Super::Destroyed();
}
