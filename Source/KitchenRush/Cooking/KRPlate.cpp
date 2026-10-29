// KRPlate.cpp

#include "Cooking/KRPlate.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRTuningData.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Hazards/KRShards.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "KitchenRushGameMode.h"

AKRPlate::AKRPlate()
{
	DisplayName = NSLOCTEXT("KitchenRush", "PlateName", "Plate");
	PrimaryActorTick.bCanEverTick = true; // [M3.5] remembers its speed so it knows how hard it hit the floor

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

void AKRPlate::AddSoup(bool bPerfect, const FKRPlatedPart& Tally)
{
	bHasSoup = true;
	bSoupPerfect = bPerfect;
	SoupTally = Tally; // [M3.7]
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
	SoupTally = FKRPlatedPart(); // [M3.7]
	SoupFill->SetVisibility(false);
}

// [M3.5]
void AKRPlate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Ticks before physics each frame, so this is the speed going INTO any hit this frame.
	LastSpeed = GetVelocity().Size();
}

// [M3.5]
void AKRPlate::OnPhysicsHit(AActor* OtherActor, const FHitResult& Hit)
{
	if (IsHeld() || IsStored() || LastSpeed < UKRTuningData::Get(this)->PlateShatterSpeed)
	{
		return;
	}
	// "The floor" = a flat surface at the height of the player's feet. Counters are 90 cm up, so they don't count.
	if (IsFloorHit(Hit)) // [M3.7] moved into AKRItem so food can use it too
	{
		Shatter(Hit.ImpactPoint);
	}
}

// [M3.5]
void AKRPlate::Shatter(const FVector& Where)
{
	// Food falls on the floor (the 5-second rule applies); soup is lost.
	for (int32 i = 0; i < Foods.Num(); ++i)
	{
		if (AKRFood* Food = Foods[i])
		{
			const float Angle = FMath::DegreesToRadians(120.f * i);
			Food->OnPlaced(Where + FVector(FMath::Cos(Angle) * 20.f, FMath::Sin(Angle) * 20.f, 8.f), FRotator::ZeroRotator);
		}
	}
	Foods.Empty(); // so Destroyed() doesn't destroy the food we just dropped

	UClass* PileClass = ShardsClass ? ShardsClass.Get() : AKRShards::StaticClass();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AKRShards* Shards = GetWorld()->SpawnActor<AKRShards>(PileClass, Where + FVector(0.f, 0.f, 3.f), FRotator::ZeroRotator, Params))
	{
		Shards->DisplayName = FText::Format(NSLOCTEXT("KitchenRush", "BrokenName", "Broken {0}"), DisplayName);
	}
	if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
	{
		GameMode->ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "Smashed", "CRASH! The {0} smashed. Mind the pieces."), DisplayName));
	}
	Destroy();
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
		Soup.TrickShots = SoupTally.TrickShots;       // [M3.7]
		Soup.GusAssists = SoupTally.GusAssists;
		Soup.GroundTouches = SoupTally.GroundTouches;
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

// [M3]
bool AKRPlate::CanReceiveItem(const AKRItem* Item) const
{
	return CanAcceptFood(Cast<AKRFood>(Item));
}

// [M3]
bool AKRPlate::ReceiveThrownItem(AKRItem* Item)
{
	return AddFood(Cast<AKRFood>(Item));
}

FText AKRPlate::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (IsInFlight()) // [M3]
	{
		return GetCatchPrompt(HeldItem);
	}
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
	if (IsInFlight()) // [M3]
	{
		return CatchBy(Interactor);
	}
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
