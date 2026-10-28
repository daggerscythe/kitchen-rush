// Copyright Epic Games, Inc. All Rights Reserved.

#include "KitchenRushGameMode.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

AKitchenRushGameMode::AKitchenRushGameMode()
{
	// GameModes counts down customer patience
	PrimaryActorTick.bCanEverTick = true;
}

void AKitchenRushGameMode::BeginPlay()
{
	Super::BeginPlay();
	SecondsUntilNextOrder = UKRTuningData::Get(this)->FirstOrderDelay;
	if (!RecipeTable)
	{
		ShowMessage(NSLOCTEXT("KitchenRush", "NoRecipes", "Set Recipe Table in BP_FirstPersonGameMode!"));
	}
}

const UKRTuningData* AKitchenRushGameMode::GetTuning() const
{
	return Tuning;
}

const FKRRecipeRow* AKitchenRushGameMode::FindRecipe(FName RowName) const
{
	return RecipeTable ? RecipeTable->FindRow<FKRRecipeRow>(RowName, TEXT("KitchenRush")) : nullptr;
}

void AKitchenRushGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bGameOver || !bOrdersEnabled)
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);

	// 1) Count down patience. Walk backwards so removing an order doesn't skip the next one.
	for (int32 i = Orders.Num() - 1; i >= 0; --i)
	{
		Orders[i].PatienceLeft -= DeltaSeconds;
		if (Orders[i].PatienceLeft <= 0.f)
		{
			const FKRRecipeRow* Recipe = FindRecipe(Orders[i].RecipeRow);
			const FText Name = Recipe ? Recipe->DisplayName : FText::FromName(Orders[i].RecipeRow);
			Orders.RemoveAt(i);
			AddStrike(FText::Format(NSLOCTEXT("KitchenRush", "WalkedOut", "A customer gave up on {0}!"), Name));
			if (bGameOver)
			{
				return;
			}
		}
	}

	// 2) Spawn new tickets. Never leave the player with nothing to do for long.
	SecondsUntilNextOrder -= DeltaSeconds;
	if (Orders.Num() == 0)
	{
		SecondsUntilNextOrder = FMath::Min(SecondsUntilNextOrder, T->FirstOrderDelay);
	}
	if (SecondsUntilNextOrder <= 0.f && Orders.Num() < T->MaxOpenTickets)
	{
		SpawnOrder();
		SecondsUntilNextOrder = T->NewOrderInterval;
	}
}

void AKitchenRushGameMode::SpawnOrder()
{
	if (!RecipeTable)
	{
		return;
	}
	// Collect every recipe that can come up at random, then pick one.
	TArray<FName> Pool;
	for (const FName& RowName : RecipeTable->GetRowNames())
	{
		const FKRRecipeRow* Recipe = FindRecipe(RowName);
		if (Recipe && Recipe->bInRandomPool)
		{
			Pool.Add(RowName);
		}
	}
	if (Pool.Num() == 0)
	{
		return;
	}
	const FName Picked = Pool[FMath::RandRange(0, Pool.Num() - 1)];
	const FKRRecipeRow* Recipe = FindRecipe(Picked);
	const UKRTuningData* T = UKRTuningData::Get(this);

	FKROrder Order;
	Order.Id = NextOrderId++;
	Order.RecipeRow = Picked;
	Order.PatienceTotal = T->PatienceBase + T->PatiencePerStep * Recipe->Steps;
	Order.PatienceLeft = Order.PatienceTotal;
	Orders.Add(Order);
}

float AKitchenRushGameMode::ScorePlate(const FKRRecipeRow& Recipe, const AKRPlate* Plate, bool& bOutPerfect) const
{
	bOutPerfect = false;
	if (Recipe.Parts.Num() == 0 || Recipe.bServedInBowl != Plate->bIsBowl)
	{
		return 0.f; // soup on a plate or salad in a bowl counts as all wrong
	}

	TArray<EKRDishPart> StillNeeded = Recipe.Parts;
	const TArray<FKRPlatedPart> OnPlate = Plate->GetPlatedParts();
	int32 Correct = 0;
	bool bAnyCooked = false;
	bool bAllCookedPerfect = true;

	for (const FKRPlatedPart& Part : OnPlate)
	{
		const int32 Index = (Part.Part == EKRDishPart::None) ? INDEX_NONE : StillNeeded.Find(Part.Part);
		if (Index != INDEX_NONE)
		{
			StillNeeded.RemoveAt(Index); // each required part can only be matched once
			++Correct;
			if (Part.bCooked)
			{
				bAnyCooked = true;
				bAllCookedPerfect &= Part.bPerfect;
			}
		}
	}

	const int32 WrongOrExtra = OnPlate.Num() - Correct;
	const UKRTuningData* T = UKRTuningData::Get(this);
	const float Score = float(Correct) / Recipe.Parts.Num() - T->WrongItemPenalty * WrongOrExtra;

	bOutPerfect = bAnyCooked && bAllCookedPerfect;
	return FMath::Clamp(Score, 0.f, 1.f);
}

bool AKitchenRushGameMode::ServeDish(AKRPlate* Plate)
{
	if (!Plate || Orders.Num() == 0 || bGameOver)
	{
		return false;
	}

	// Find the ticket this dish matches best. Ties go to the most impatient customer.
	int32 BestIndex = INDEX_NONE;
	float BestScore = -1.f;
	bool bBestPerfect = false;
	for (int32 i = 0; i < Orders.Num(); ++i)
	{
		const FKRRecipeRow* Recipe = FindRecipe(Orders[i].RecipeRow);
		if (!Recipe)
		{
			continue;
		}
		bool bPerfect = false;
		const float Score = ScorePlate(*Recipe, Plate, bPerfect);
		const bool bBetter = Score > BestScore
			|| (FMath::IsNearlyEqual(Score, BestScore) && BestIndex != INDEX_NONE && Orders[i].PatienceLeft < Orders[BestIndex].PatienceLeft);
		if (bBetter)
		{
			BestIndex = i;
			BestScore = Score;
			bBestPerfect = bPerfect;
		}
	}
	if (BestIndex == INDEX_NONE)
	{
		return false;
	}

	const FKROrder Order = Orders[BestIndex];
	Orders.RemoveAt(BestIndex);
	const FKRRecipeRow* Recipe = FindRecipe(Order.RecipeRow);
	const UKRTuningData* T = UKRTuningData::Get(this);

	if (BestScore < T->BadOrderThreshold)
	{
		AddStrike(FText::Format(NSLOCTEXT("KitchenRush", "BadOrder", "That's not {0}!"), Recipe->DisplayName));
		return true;
	}

	const int32 Base = FMath::RoundToInt(T->TipBase * BestScore);
	const int32 Perfect = bBestPerfect ? FMath::RoundToInt(T->PerfectBonus) : 0;
	const int32 Speed = FMath::RoundToInt(T->SpeedBonusMax * FMath::Clamp(Order.PatienceLeft / Order.PatienceTotal, 0.f, 1.f));
	const int32 Earned = Base + Perfect + Speed;
	Tips += Earned;

	ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "Served", "+${0}  {1}  ({2}% correct{3})"),
		Earned, Recipe->DisplayName, FMath::RoundToInt(BestScore * 100.f),
		bBestPerfect ? NSLOCTEXT("KitchenRush", "PerfectTag", ", perfect cook!") : FText::GetEmpty()));
	return true;
}

void AKitchenRushGameMode::AddStrike(const FText& Reason)
{
	++Strikes;
	const int32 MaxStrikes = UKRTuningData::Get(this)->MaxStrikes;
	if (Strikes >= MaxStrikes)
	{
		bGameOver = true;
		Orders.Empty();
		// M4 replaces this with the Game Over screen.
		ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "GameOver", "{0}  Strike {1}: GAME OVER"), Reason, Strikes));
		return;
	}
	ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "Strike", "{0}  Strike {1}/{2}"), Reason, Strikes, MaxStrikes));
}

void AKitchenRushGameMode::ShowMessage(const FText& Text)
{
	Message = Text;
	MessageExpiresAt = GetWorld()->GetRealTimeSeconds() + 3.0;
}

FText AKitchenRushGameMode::GetMessage() const
{
	return (GetWorld()->GetRealTimeSeconds() < MessageExpiresAt) ? Message : FText::GetEmpty();
}
