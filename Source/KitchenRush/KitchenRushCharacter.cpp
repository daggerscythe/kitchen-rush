// Copyright Epic Games, Inc. All Rights Reserved.

#include "KitchenRushCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/KRInteractionComponent.h"
#include "Time/KRTimeSubsystem.h"
#include "Engine/World.h"
#include "KitchenRush.h"

AKitchenRushCharacter::AKitchenRushCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Hold point: a child of the camera, it moves and turns with view
	// X = forward, Y = right, Z = up (in cm), relative to the camera
	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Hold Point"));
	HoldPoint->SetupAttachment(FirstPersonCameraComponent);
	HoldPoint->SetRelativeLocation(FVector(45.f, 18.f, -22.f));

	// Interaction component
	Interaction = CreateDefaultSubobject<UKRInteractionComponent>(TEXT("Interaction"));

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;

	// Crouching shrinks the capsule to CrouchedHalfHeight and lowers the mesh
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(60.0f);   // standing half height is 96
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;  // 2 m/s
}

void AKitchenRushCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	SpawnRotation = GetActorRotation();

	// Remember the standing setup for the crouch camera.
	FirstPersonMeshBaseLocation = FirstPersonMesh->GetRelativeLocation();
	StandingHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	Interaction->Setup(FirstPersonCameraComponent, HoldPoint);

	// Frozen look: set up the cameras post-process settings
	FPostProcessSettings& PP = FirstPersonCameraComponent->PostProcessSettings;
	PP.bOverride_ColorSaturation = true;
	PP.ColorSaturation = FVector4(FrozenSaturation, FrozenSaturation, FrozenSaturation, 1.f);
	PP.bOverride_ColorGain = true;
	PP.ColorGain = FVector4(FrozenTint.R, FrozenTint.G, FrozenTint.B, 1.f);
	PP.bOverride_VignetteIntensity = true;
	PP.VignetteIntensity = FrozenVignette;
	FirstPersonCameraComponent->PostProcessBlendWeight = 0.f;
}

void AKitchenRushCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (const UKRTimeSubsystem* Time = GetWorld()->GetSubsystem<UKRTimeSubsystem>())
	{
		FirstPersonCameraComponent->PostProcessBlendWeight = Time->GetFreezeAmount();
	}

	// Crouch camera
	const float CrouchDrop = StandingHalfHeight - GetCharacterMovement()->GetCrouchedHalfHeight();
	const float TargetOffset = bIsCrouched ? -CrouchDrop : 0.f;
	CrouchCameraOffset = FMath::FInterpTo(CrouchCameraOffset, TargetOffset, DeltaSeconds, CrouchCameraSpeed);
	FirstPersonMesh->SetRelativeLocation(FirstPersonMeshBaseLocation + FVector(0.f, 0.f, CrouchCameraOffset));
}

void AKitchenRushCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	// Put the player back at the start
	GetCharacterMovement()->StopMovementImmediately();
	SetActorLocationAndRotation(SpawnLocation, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AKitchenRushCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AKitchenRushCharacter::DoJumpStart);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AKitchenRushCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKitchenRushCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKitchenRushCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AKitchenRushCharacter::LookInput);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AKitchenRushCharacter::DoInteract);

		// Crouching: hold to crouch, release to stand
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AKitchenRushCharacter::DoCrouchStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AKitchenRushCharacter::DoCrouchEnd);
	}
	else
	{
		UE_LOG(LogKitchenRush, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AKitchenRushCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AKitchenRushCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AKitchenRushCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AKitchenRushCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AKitchenRushCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AKitchenRushCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AKitchenRushCharacter::DoInteract()
{
	Interaction->Interact();
}

void AKitchenRushCharacter::DoCrouchStart()
{
	Crouch();   // built into ACharacter; does nothing if crouching isn't allowed
}

void AKitchenRushCharacter::DoCrouchEnd()
{
	UnCrouch(); // stays crouched automatically if something is above your head
}
