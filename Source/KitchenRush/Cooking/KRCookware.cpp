// KRCookware.cpp

#include "Cooking/KRCookware.h"
#include "Cooking/KRStove.h"
#include "Cooking/KRTuningData.h"
#include "UI/KRMeterWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "KitchenRushGameMode.h"

AKRCookware::AKRCookware()
{
	// Cookware needs to run every frame (AKRItem turned ticking off).
	PrimaryActorTick.bCanEverTick = true;
	// The pan and pot can now be carried and thrown.

	// Draws a UMG widget in the 3D world
	Meter = CreateDefaultSubobject<UWidgetComponent>(TEXT("Meter"));
	Meter->SetupAttachment(Mesh);
	Meter->SetWidgetSpace(EWidgetSpace::Screen); // keeps it facing the camera at a constant size
	Meter->SetDrawSize(FVector2D(180.f, 40.f));
	Meter->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
	Meter->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AKRCookware::IsOnBurner() const
{
	if (IsHeld())
	{
		return false;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	return AKRStove::IsNearBurner(GetWorld(), GetActorLocation(), T->BurnerRadius, T->BurnerHeight);
}

void AKRCookware::OnHitByThrownItem(AKRItem* Item)
{
	if (!Item || !Mesh->IsSimulatingPhysics())
	{
		return;
	}
	// Push away from where the item hit us (flat), plus a little hop so it clears the burner.
	// (We don't use the item's velocity: by the time we hear about the hit, it has already bounced.)
	FVector Direction = (GetActorLocation() - Item->GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}
	const bool bWasOnBurner = IsOnBurner();
	const float Speed = UKRTuningData::Get(this)->KnockSpeed;
	// bVelChange = true: "add this much speed", whatever the mesh's mass is.
	Mesh->AddImpulse(Direction * Speed + FVector(0.f, 0.f, Speed * 0.3f), NAME_None, true);

	if (bWasOnBurner)
	{
		if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
		{
			GameMode->ShowMessage(FText::Format(NSLOCTEXT("KitchenRush", "KnockedOff", "Knocked the {0} off the burner!"), DisplayName));
		}
	}
}

void AKRCookware::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// food stops cooking when freezed
	if (IsOnBurner())
	{
		Cook(DeltaSeconds);
	}
	UpdateMeter();
}

void AKRCookware::SetMeter(bool bVisible, float Percent, const FLinearColor& Color, const FText& Label)
{
	Meter->SetVisibility(bVisible);
	if (UKRMeterWidget* Widget = Cast<UKRMeterWidget>(Meter->GetUserWidgetObject()))
	{
		Widget->SetMeter(Percent, Color, Label);
	}
}
