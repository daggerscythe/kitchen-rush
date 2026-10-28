// KRCookware.cpp

#include "Cooking/KRCookware.h"
#include "Cooking/KRStove.h"
#include "Cooking/KRTuningData.h"
#include "UI/KRMeterWidget.h"
#include "Components/WidgetComponent.h"

AKRCookware::AKRCookware()
{
	// Cookware needs to run every frame (AKRItem turned ticking off).
	PrimaryActorTick.bCanEverTick = true;
	bCanBePickedUp = false;

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
