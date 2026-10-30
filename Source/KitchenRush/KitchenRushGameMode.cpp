// Copyright Epic Games, Inc. All Rights Reserved.

#include "KitchenRushGameMode.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Flow/KRProfileSubsystem.h"
#include "KitchenRushPlayerController.h"

AKitchenRushGameMode::AKitchenRushGameMode()
{
	// GameModes counts down customer patience
	PrimaryActorTick.bCanEverTick = true;
}

void AKitchenRushGameMode::BeginPlay()
{
	Super::BeginPlay();
	SecondsUntilNextOrder = UKRTuningData::Get(this)->FirstOrderDelay;

	// [M4] Saved settings (time modes, UI scale) apply to every level.
	if (const UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->ApplySettings(GetWorld());
	}

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
	if (bGameOver)
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);

	// 1) Count down patience. Walk backwards so removing an order doesn't skip the next one.
	// [M4] This runs even when random orders are off, so the act director's own tickets count down too.
	for (int32 i = Orders.Num() - 1; i >= 0; --i)
	{
		if (Orders[i].PatienceTotal <= 0.f)
		{
			continue; // [M4] a ticket without a patience bar never runs out
		}
		Orders[i].PatienceLeft -= DeltaSeconds;
		if (Orders[i].PatienceLeft <= 0.f && bTutorialMode)
		{
			Orders[i].PatienceLeft = Orders[i].PatienceTotal; // [M4] the tutorial is forgiving: just refill
			ShowMessage(NSLOCTEXT("KitchenRush", "TutorialPatience", "Out of patience! In the real rush that's a strike. Try again."));
			continue;
		}
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
	if (!bOrdersEnabled)
	{
		return; // [M4]
	}
	SecondsUntilNextOrder -= DeltaSeconds;
	if (Orders.Num() == 0)
	{
		SecondsUntilNextOrder = FMath::Min(SecondsUntilNextOrder, T->FirstOrderDelay);
	}
	if (SecondsUntilNextOrder <= 0.f && Orders.Num() < GetMaxTickets())
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
	// [M3.5] Was a plain random pick (Garden Salad 4 times in a row could happen). Now a shuffle bag.
	const FName Picked = DrawRecipe();
	const FKRRecipeRow* Recipe = FindRecipe(Picked);
	if (!Recipe)
	{
		return;
	}
	AddOrder(Picked, 0.f); // [M4] the normal patience formula
}

// [M4]
void AKitchenRushGameMode::AddOrder(FName RecipeRow, float Patience)
{
	const FKRRecipeRow* Recipe = FindRecipe(RecipeRow);
	if (!Recipe)
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);

	FKROrder Order;
	Order.Id = NextOrderId++;
	Order.RecipeRow = RecipeRow;
	if (Patience < 0.f)
	{
		Order.PatienceTotal = 0.f; // no patience bar
	}
	else if (Patience > 0.f)
	{
		Order.PatienceTotal = Patience;
	}
	else
	{
		Order.PatienceTotal = T->PatienceBase + T->PatiencePerStep * Recipe->Steps;
	}
	Order.PatienceLeft = Order.PatienceTotal;
	Orders.Add(Order);
}

// [M4]
int32 AKitchenRushGameMode::GetMaxTickets() const
{
	return MaxTicketsOverride > 0 ? MaxTicketsOverride : UKRTuningData::Get(this)->MaxOpenTickets;
}

// [M4]
void AKitchenRushGameMode::SetMenu(const TArray<FName>& RecipeRows)
{
	Menu = RecipeRows;
	RecipeBag.Empty(); // reshuffle with the new menu
}

// [M4]
void AKitchenRushGameMode::AddToMenu(FName RecipeRow)
{
	Menu.AddUnique(RecipeRow);
	RecipeBag.Empty();
}

// [M4]
void AKitchenRushGameMode::BroadcastEvent(EKRGameEvent Type, FName Name, float Value, int32 Flags)
{
	FKRGameEvent Event;
	Event.Type = Type;
	Event.Name = Name;
	Event.Value = Value;
	Event.Flags = Flags;
	OnGameEvent.Broadcast(Event);
}

// [M4]
void AKitchenRushGameMode::NotifyGameEvent(const UObject* WorldContext, EKRGameEvent Type, FName Name, float Value, int32 Flags)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (AKitchenRushGameMode* GameMode = World ? World->GetAuthGameMode<AKitchenRushGameMode>() : nullptr)
	{
		GameMode->BroadcastEvent(Type, Name, Value, Flags);
	}
}

// [M3.5] A "shuffle bag", like a deck of cards: put one of each dish in, shuffle, deal from the top,
// and only reshuffle when it's empty. Random, but you can't get the same dish over and over.
FName AKitchenRushGameMode::DrawRecipe()
{
	if (RecipeBag.Num() == 0)
	{
		// [M4] The act's menu if there is one, otherwise every recipe in the random pool.
		if (Menu.Num() > 0)
		{
			RecipeBag = Menu;
		}
		else
		{
			for (const FName& RowName : RecipeTable->GetRowNames())
			{
				const FKRRecipeRow* Recipe = FindRecipe(RowName);
				if (Recipe && Recipe->bInRandomPool)
				{
					RecipeBag.Add(RowName);
				}
			}
		}
		if (RecipeBag.Num() == 0)
		{
			return NAME_None;
		}
		// Fisher-Yates shuffle: swap each slot with a random slot at or after it.
		for (int32 i = 0; i < RecipeBag.Num() - 1; ++i)
		{
			RecipeBag.Swap(i, FMath::RandRange(i, RecipeBag.Num() - 1));
		}
		// The bag is dealt from the end: don't start the new bag with the dish we just had.
		if (RecipeBag.Num() > 1 && RecipeBag.Last() == LastRecipe)
		{
			RecipeBag.Swap(RecipeBag.Num() - 1, 0);
		}
	}

	// Prefer a dish that isn't already on the rail.
	int32 Pick = RecipeBag.Num() - 1;
	for (int32 i = RecipeBag.Num() - 1; i >= 0; --i)
	{
		const FName Candidate = RecipeBag[i];
		if (!Orders.ContainsByPredicate([Candidate](const FKROrder& Order) { return Order.RecipeRow == Candidate; }))
		{
			Pick = i;
			break;
		}
	}
	LastRecipe = RecipeBag[Pick];
	RecipeBag.RemoveAt(Pick);
	return LastRecipe;
}

// [M3.7] Rewritten: returns everything the receipt needs in one struct.
FKRPlateScore AKitchenRushGameMode::ScorePlate(const FKRRecipeRow& Recipe, const AKRPlate* Plate) const
{
	FKRPlateScore Result;
	if (Recipe.Parts.Num() == 0 || Recipe.bServedInBowl != Plate->bIsBowl)
	{
		return Result; // soup on a plate or salad in a bowl counts as all wrong
	}

	TArray<EKRDishPart> StillNeeded = Recipe.Parts;
	const TArray<FKRPlatedPart> OnPlate = Plate->GetPlatedParts();
	int32 Correct = 0;
	bool bAnyCooked = false;
	bool bAllCookedPerfect = true;

	for (const FKRPlatedPart& Part : OnPlate)
	{
		// Anything served that touched the floor costs money, right or wrong.
		Result.GroundTouches += Part.GroundTouches;

		const int32 Index = (Part.Part == EKRDishPart::None) ? INDEX_NONE : StillNeeded.Find(Part.Part);
		if (Index != INDEX_NONE)
		{
			StillNeeded.RemoveAt(Index); // each required part can only be matched once
			++Correct;
			// Bonuses only count for parts that belong in the dish.
			Result.TrickShots += Part.TrickShots;
			Result.GusAssists += Part.GusAssists;
			if (Part.bCooked)
			{
				bAnyCooked = true;
				bAllCookedPerfect &= Part.bPerfect;
			}
		}
	}

	const int32 WrongOrExtra = OnPlate.Num() - Correct;
	const UKRTuningData* T = UKRTuningData::Get(this);
	Result.Correctness = FMath::Clamp(float(Correct) / Recipe.Parts.Num() - T->WrongItemPenalty * WrongOrExtra, 0.f, 1.f);
	Result.bPerfectOrder = (Correct == Recipe.Parts.Num() && WrongOrExtra == 0);
	Result.bPerfectCook = bAnyCooked && bAllCookedPerfect;
	return Result;
}

// [M3.7] Adds a "  - Perfect cook! (+$2)" style line and returns the dollars, so totals and lines can't disagree.
static int32 AddReceiptLine(FKRReceipt& Receipt, EKRReceiptLine Kind, const FText& Label, int32 Dollars)
{
	FKRReceiptLine Line;
	Line.Kind = Kind;
	Line.Text = (Kind == EKRReceiptLine::Penalty)
		? FText::Format(NSLOCTEXT("KitchenRush", "ReceiptPenalty", "- {0} (-${1})"), Label, Dollars)
		: FText::Format(NSLOCTEXT("KitchenRush", "ReceiptBonus", "- {0} (+${1})"), Label, Dollars);
	Receipt.Lines.Add(Line);
	return (Kind == EKRReceiptLine::Penalty) ? -Dollars : Dollars;
}

bool AKitchenRushGameMode::ServeDish(AKRPlate* Plate, bool bThrownIntoPass)
{
	if (!Plate || Orders.Num() == 0 || bGameOver)
	{
		return false;
	}

	// Find the ticket this dish matches best. Ties go to the most impatient customer.
	int32 BestIndex = INDEX_NONE;
	FKRPlateScore Best;
	Best.Correctness = -1.f;
	for (int32 i = 0; i < Orders.Num(); ++i)
	{
		const FKRRecipeRow* Recipe = FindRecipe(Orders[i].RecipeRow);
		if (!Recipe)
		{
			continue;
		}
		const FKRPlateScore Score = ScorePlate(*Recipe, Plate);
		const bool bBetter = Score.Correctness > Best.Correctness
			|| (FMath::IsNearlyEqual(Score.Correctness, Best.Correctness) && BestIndex != INDEX_NONE && Orders[i].PatienceLeft < Orders[BestIndex].PatienceLeft);
		if (bBetter)
		{
			BestIndex = i;
			Best = Score;
		}
	}
	if (BestIndex == INDEX_NONE)
	{
		return false;
	}

	const UKRTuningData* T = UKRTuningData::Get(this);
	const FKRRecipeRow* Recipe = FindRecipe(Orders[BestIndex].RecipeRow);

	// [M4] Tutorial: hand a wrong dish back instead of taking it (no strike, the ticket stays).
	if (bTutorialMode && Best.Correctness < 1.f)
	{
		ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "TutorialWrong", "Not quite. Check the ticket: {0}."), Recipe->DisplayName));
		BroadcastEvent(EKRGameEvent::DishRejected, Orders[BestIndex].RecipeRow);
		return false;
	}

	const FKROrder Order = Orders[BestIndex];
	Orders.RemoveAt(BestIndex);

	if (Best.Correctness < T->BadOrderThreshold)
	{
		AddStrike(FText::Format(NSLOCTEXT("KitchenRush", "BadOrder", "That's not {0}!"), Recipe->DisplayName));
		return true;
	}

	// ---- [M3.7] Build the receipt, line by line. ----
	FKRReceipt Receipt;
	Receipt.Id = NextReceiptId++;

	// Header: the dish's price, scaled by how correct it was.
	const int32 Price = Recipe->Price > 0 ? Recipe->Price : FMath::RoundToInt(T->TipBase);
	const int32 Base = FMath::RoundToInt(Price * Best.Correctness);
	FKRReceiptLine Header;
	Header.Kind = EKRReceiptLine::Header;
	Header.Text = Best.bPerfectOrder
		? FText::Format(NSLOCTEXT("KitchenRush", "ReceiptHeader", "+${0}  {1}"), Base, Recipe->DisplayName)
		: FText::Format(NSLOCTEXT("KitchenRush", "ReceiptHeaderPct", "+${0}  {1} ({2}% correct)"), Base, Recipe->DisplayName, FMath::RoundToInt(Best.Correctness * 100.f));
	Receipt.Lines.Add(Header);
	int32 Total = Base;

	if (Best.bPerfectOrder && T->PerfectOrderBonus > 0.f)
	{
		Total += AddReceiptLine(Receipt, EKRReceiptLine::Bonus, NSLOCTEXT("KitchenRush", "PerfectOrder", "Perfect order!"), FMath::RoundToInt(T->PerfectOrderBonus));
	}
	if (Best.bPerfectCook && T->PerfectBonus > 0.f)
	{
		Total += AddReceiptLine(Receipt, EKRReceiptLine::Bonus, NSLOCTEXT("KitchenRush", "PerfectCook", "Perfect cook!"), FMath::RoundToInt(T->PerfectBonus));
	}

	// Trick shots: the ingredients' throws, plus one if the plate itself was thrown through the pass.
	const int32 Tricks = Best.TrickShots + (bThrownIntoPass ? 1 : 0);
	if (Tricks > 0 && T->TrickShotBonus > 0.f)
	{
		const FText Label = (Tricks == 1)
			? NSLOCTEXT("KitchenRush", "OneTrick", "1 trick shot!")
			: FText::Format(NSLOCTEXT("KitchenRush", "Tricks", "{0} trick shots!"), Tricks);
		Total += AddReceiptLine(Receipt, EKRReceiptLine::Bonus, Label, Tricks * FMath::RoundToInt(T->TrickShotBonus));
	}
	if (Best.GusAssists > 0 && T->GusAssistBonus > 0.f)
	{
		const FText Label = (Best.GusAssists == 1)
			? NSLOCTEXT("KitchenRush", "OneAssist", "Gus Assist!")
			: FText::Format(NSLOCTEXT("KitchenRush", "Assists", "{0} Gus Assists!"), Best.GusAssists);
		Total += AddReceiptLine(Receipt, EKRReceiptLine::Bonus, Label, Best.GusAssists * FMath::RoundToInt(T->GusAssistBonus));
	}

	// Patience: a flat bonus if the customer's bar was still at least this full.
	const float PatienceLeft = Order.PatienceTotal > 0.f ? Order.PatienceLeft / Order.PatienceTotal : 0.f;
	if (PatienceLeft >= T->PatienceBonusThreshold && T->PatienceBonus > 0.f)
	{
		Total += AddReceiptLine(Receipt, EKRReceiptLine::Bonus, NSLOCTEXT("KitchenRush", "Patience", "Patience bonus!"), FMath::RoundToInt(T->PatienceBonus));
	}

	if (Best.GroundTouches > 0 && T->GroundPenalty > 0.f)
	{
		const FText Label = (Best.GroundTouches == 1)
			? NSLOCTEXT("KitchenRush", "OneGround", "1 item touched the ground")
			: FText::Format(NSLOCTEXT("KitchenRush", "Ground", "{0} items touched the ground"), Best.GroundTouches);
		Total += AddReceiptLine(Receipt, EKRReceiptLine::Penalty, Label, Best.GroundTouches * FMath::RoundToInt(T->GroundPenalty));
	}

	Total = FMath::Max(Total, 0); // penalties can eat the tip, but never take money away
	FKRReceiptLine TotalLine;
	TotalLine.Kind = EKRReceiptLine::Total;
	TotalLine.Text = FText::Format(NSLOCTEXT("KitchenRush", "ReceiptTotal", "Total: ${0}"), Total);
	Receipt.Lines.Add(TotalLine);

	Tips += Total;
	++Served; // [M4]
	Receipts.Add(Receipt);
	if (Receipts.Num() > 10)
	{
		Receipts.RemoveAt(0); // the HUD only needs the recent ones
	}
	BroadcastEvent(EKRGameEvent::DishServed, Order.RecipeRow, Best.Correctness); // [M4]
	return true;
}

void AKitchenRushGameMode::AddStrike(const FText& Reason)
{
	if (bTutorialMode)
	{
		return; // [M4] no strikes while Nana is teaching
	}
	++Strikes;
	BroadcastEvent(EKRGameEvent::Strike); // [M4]
	const int32 MaxStrikes = UKRTuningData::Get(this)->MaxStrikes;
	if (Strikes >= MaxStrikes)
	{
		TriggerGameOver(FText::Format(NSLOCTEXT("KitchenRush", "StrikeOut", "{0}  Strike {1}!"), Reason, Strikes), EKRRunOutcome::Strikes);
		return;
	}
	ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "Strike", "{0}  Strike {1}/{2}"), Reason, Strikes, MaxStrikes));
}

void AKitchenRushGameMode::ShowMessage(const FText& Text, float Seconds)
{
	Message = Text;
	MessageExpiresAt = GetWorld()->GetRealTimeSeconds() + Seconds;
}

// [M3]
void AKitchenRushGameMode::AddBonus(int32 Dollars, const FText& Reason)
{
	if (bGameOver || Dollars <= 0)
	{
		return;
	}
	Tips += Dollars;
	ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "Bonus", "+${0}  {1}"), Dollars, Reason));
}

// [M3] [M4] now a thin wrapper: losing is just EndShift without winning.
void AKitchenRushGameMode::TriggerGameOver(const FText& Reason, EKRRunOutcome Outcome)
{
	EndShift(false, Reason, Outcome);
}

// [M4]
void AKitchenRushGameMode::EndShift(bool bWon, const FText& Reason, EKRRunOutcome Outcome)
{
	if (bGameOver)
	{
		return;
	}
	bGameOver = true;
	Orders.Empty();

	const UKRTuningData* T = UKRTuningData::Get(this);
	FKRResults Results;
	Results.bWon = bWon;
	Results.Reason = Reason;
	Results.Tips = Tips;
	Results.Served = Served;
	Results.Strikes = Strikes;
	// Stars: surviving the night is 1; the tip thresholds (set after playtest 2) add the others.
	if (bWon)
	{
		Results.Stars = 1 + (Tips >= T->TwoStarTips ? 1 : 0) + (Tips >= T->ThreeStarTips ? 1 : 0);
	}

	// Only a full night (started at Act 1) can set the high score.
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Results.bNewBest = bFullRun && Profile->RecordTips(Tips, bWon);
		Results.Best = Profile->GetHighScore();
	}

	BroadcastEvent(EKRGameEvent::RunEnded, NAME_None, float(Tips), int32(Outcome));

	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ShowResults(Results);
	}
}

FText AKitchenRushGameMode::GetMessage() const
{
	return (GetWorld()->GetRealTimeSeconds() < MessageExpiresAt) ? Message : FText::GetEmpty();
}
