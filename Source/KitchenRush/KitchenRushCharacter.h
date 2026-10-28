// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "KitchenRushCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class USceneComponent;
class UKRInteractionComponent;
class UDamageType;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class AKitchenRushCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Where a carried item sits, in front of the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HoldPoint;

	/** Finds what the player is looking at, shows prompts, handles E, remembers the held item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UKRInteractionComponent* Interaction;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Interact Input Action (E) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	/** Crouch Input Action (hold Left Ctrl) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	/** Frozen look: colour saturation when fully frozen (1 = normal colour, 0 = grey) */
	UPROPERTY(EditAnywhere, Category = "Time|Frozen Look", meta = (ClampMin = "0", ClampMax = "1"))
	float FrozenSaturation = 0.35f;

	/** Frozen look: colour multiplier when fully frozen (slightly blue) */
	UPROPERTY(EditAnywhere, Category = "Time|Frozen Look")
	FLinearColor FrozenTint = FLinearColor(0.9f, 0.97f, 1.15f, 1.f);

	/** Frozen look: dark edges when fully frozen */
	UPROPERTY(EditAnywhere, Category = "Time|Frozen Look", meta = (ClampMin = "0", ClampMax = "1"))
	float FrozenVignette = 0.6f;
	
public:
	AKitchenRushCharacter();

protected:

	/** Runs once when the game starts */
	virtual void BeginPlay() override;

	/** Runs every frame */
	virtual void Tick(float DeltaSeconds) override;

	/** Called when the player drops below Kill Z */
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Handles the interact input (E) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoInteract();

	/** Start crouching (Ctrl pressed) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoCrouchStart();

	/** Stop crouching (Ctrl released) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoCrouchEnd();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns the interaction component **/
	UKRInteractionComponent* GetInteraction() const { return Interaction; }

private:
	/** Where the player started, for respawning after falling out of the world */
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
};

