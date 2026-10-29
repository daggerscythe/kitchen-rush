// KRInteractionComponent.h
// Lives on the player. Every frame it fires a line trace out of the camera to see what you're
// looking at, works out the prompt text, and when you press E it does the interaction.
// It also remembers the item in your hands.

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

	/** The character calls this in BeginPlay to hand over its camera and hold point. */
	void Setup(UCameraComponent* InCamera, USceneComponent* InHoldPoint);

	/** Called when the player presses E. */
	void Interact();

	/** Put an item in the player's hands. Fails if they're already holding something. */
	bool TryPickUp(AKRItem* Item);

	/**
	 * Empty the player's hands and return what was in them.
	 * The caller MUST then do something with the item (store it, destroy it...).
	 */
	AKRItem* ReleaseHeldItem();

	/** Text for the HUD. Empty when there's nothing to do. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetPrompt() const { return Prompt; }

	/** What the player is carrying (nullptr = empty hands). */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AKRItem* GetHeldItem() const;

	/** [M3.5] What the player is looking at with something to do (nullptr = nothing). The trash lid uses it. */
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** How far you can reach, in cm. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float Reach = 250.f;

	/** A surface counts as "flat enough to place on" when its normal's Z is at least this (1 = perfectly flat). */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0", ClampMax = "1"))
	float MinPlaceSurfaceUp = 0.7f;

private:
	/** Runs the line trace and fills in FocusedActor, bCanPlace, PlaceLocation and Prompt. */
	void UpdateFocus();
	void PlaceHeldItem();

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
