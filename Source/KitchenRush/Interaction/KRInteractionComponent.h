// KRInteractionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KRInteractionComponent.generated.h"

class AKRItem;
class UCameraComponent;

UCLASS(ClassGroup = (KitchenRush), meta = (BlueprintSpawnableComponent))
class KITCHENRUSH_API UKRInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKRInteractionComponent();

	/** The character calls this in BeginPlay to hand over its camera and hold point */
	void Setup(UCameraComponent* InCamera, USceneComponent* InHoldPoint);

	/** Called when the player presses E */
	void Interact();

	/** Put an item in the player's hands. Fails if they're already holding something */
	bool TryPickUp(AKRItem* Item);

	/** Text for the HUD, empty when there's nothing to do */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetPrompt() const { return Prompt; }

	/** What the player is carrying (nullptr = empty hands) */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AKRItem* GetHeldItem() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** How far you can reach, in cm */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float Reach = 250.f;

	/** A surface counts as "flat enough to place on" when its normal's Z is at least this (1 = perfectly flat) */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0", ClampMax = "1"))
	float MinPlaceSurfaceUp = 0.7f;

private:
	/** Runs the line trace and fills in FocusedActor, bCanPlace, PlaceLocation and Prompt */
	void UpdateFocus();
	void PlaceHeldItem();

	// UPROPERTY() on object pointers lets Unreal's garbage collector know we're using them
	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<USceneComponent> HoldPoint;

	UPROPERTY()
	TObjectPtr<AKRItem> HeldItem;

	TWeakObjectPtr<AActor> FocusedActor; // weak = doesn't keep the actor alive
	bool bCanPlace = false;
	FVector PlaceLocation = FVector::ZeroVector;
	FText Prompt;
};
