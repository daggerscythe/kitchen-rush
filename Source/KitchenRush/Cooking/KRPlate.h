// KRPlate.h 
// A plate or a bowl. Plates hold up to MaxPlateItems pieces of food;
// bowls hold one serving of soup.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/KRItem.h"
#include "Cooking/KRTypes.h"
#include "KRPlate.generated.h"

class AKRFood;
class AKRShards;

UCLASS()
class KITCHENRUSH_API AKRPlate : public AKRItem
{
	GENERATED_BODY()

public:
	AKRPlate();

	/** Bowls only take soup (from the pot); plates take anything else. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
	bool bIsBowl = false;

	bool CanAcceptFood(const AKRFood* Food) const;
	bool AddFood(AKRFood* Food);

	bool CanAcceptSoup() const;
	/** Tally = the tomatoes' trick shots / Gus Assists / ground touches, added up by the pot. */
	void AddSoup(bool bPerfect, const FKRPlatedPart& Tally);

	/** Destroys everything on the plate (used by the trash). */
	void ClearContents();
	bool IsEmpty() const;

	/** Everything on the plate as recipe parts, for scoring. */
	TArray<FKRPlatedPart> GetPlatedParts() const;

	/** "Chopped Tomato, Perfect Steak" or "empty". */
	FText GetContentsText() const;

	virtual FText GetItemName() const override;
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

	// Food thrown onto this plate, or caught while holding it, lands on it.
	virtual bool CanReceiveItem(const AKRItem* Item) const override;
	virtual bool ReceiveThrownItem(AKRItem* Item) override;
	virtual uint8 GetTrickShotFlag() const override { return KRTrick::Plate; }
	virtual EKRSound GetImpactSound() const override { return EKRSound::DishClink; }
	virtual EKRSound GetSetDownSound() const override { return EKRSound::DishClink; }

	virtual void Tick(float DeltaSeconds) override;

	/** Break: drop the food loose, leave a pile of shards, and destroy the plate. */
	void Shatter(const FVector& Where);

protected:
	/** Hitting the floor fast enough shatters the plate. */
	virtual void OnPhysicsHit(AActor* OtherActor, const FHitResult& Hit) override;

	/** The pile left behind when this breaks: set to BP_Shards. (Empty = invisible shards that still hurt.) */
	UPROPERTY(EditAnywhere, Category = "Plate")
	TSubclassOf<AKRShards> ShardsClass;

	/** Bowl only: the soup surface, hidden until filled. Pick its mesh in BP_Bowl. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plate")
	TObjectPtr<UStaticMeshComponent> SoupFill;

	/** When the plate is destroyed (served or trashed), destroy the food on it too. */
	virtual void Destroyed() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<AKRFood>> Foods;

	bool bHasSoup = false;
	bool bSoupPerfect = false;
	FKRPlatedPart SoupTally;
	float LastSpeed = 0.f;     // speed last frame, before any bounce
};
