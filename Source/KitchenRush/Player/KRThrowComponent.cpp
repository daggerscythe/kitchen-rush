// KRThrowComponent.cpp

#include "Player/KRThrowComponent.h"
#include "Interaction/KRInteractionComponent.h"
#include "Interaction/KRItem.h"
#include "Cooking/KRTuningData.h"
#include "Time/KRTimeSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UKRThrowComponent::UKRThrowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKRThrowComponent::Setup(UCameraComponent* InCamera, UKRInteractionComponent* InInteraction, UInstancedStaticMeshComponent* InArcDots)
{
	Camera = InCamera;
	Interaction = InInteraction;
	ArcDots = InArcDots;
}

FVector UKRThrowComponent::GetThrowVelocity() const
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	return Camera->GetForwardVector() * T->ThrowSpeed + FVector(0.f, 0.f, T->ThrowUpSpeed);
}

void UKRThrowComponent::StartAim()
{
	bAiming = Camera && Interaction && Interaction->GetHeldItem();
}

void UKRThrowComponent::ReleaseThrow()
{
	if (!bAiming)
	{
		return;
	}
	bAiming = false;
	ClearArc();

	AKRItem* Item = Interaction ? Interaction->GetHeldItem() : nullptr;
	if (!Item)
	{
		return;
	}
	const FVector Start = Item->GetActorLocation();
	const FVector Velocity = GetThrowVelocity();
	Interaction->ReleaseHeldItem();
	Item->OnThrown(Start, Velocity, false);

	if (UKRTimeSubsystem* Time = GetWorld()->GetSubsystem<UKRTimeSubsystem>())
	{
		Time->TriggerActionBurst(); // a throw costs time, like any action
	}
}

void UKRThrowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Lost the item while aiming (put it on a plate, etc.)? Stop aiming.
	if (bAiming && (!Interaction || !Interaction->GetHeldItem()))
	{
		bAiming = false;
		ClearArc();
	}
	if (bAiming)
	{
		UpdateArc();
	}
}

void UKRThrowComponent::UpdateArc()
{
	if (!ArcDots)
	{
		return;
	}
	AKRItem* Item = Interaction->GetHeldItem();

	// Ask the engine to simulate the flight and report the points along the way.
	FPredictProjectilePathParams Params(5.f, Item->GetActorLocation(), GetThrowVelocity(), ArcSeconds);
	Params.bTraceWithCollision = true;
	Params.TraceChannel = ECC_Visibility;
	Params.SimFrequency = DotsPerSecond;
	Params.ActorsToIgnore.Add(GetOwner());
	Params.ActorsToIgnore.Add(Item);
	Params.OverrideGravityZ = 0.f; // 0 = use the world's gravity, same as the real throw

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(this, Params, Result);

	ArcDots->ClearInstances();
	for (int32 i = 1; i < Result.PathData.Num(); ++i) // skip point 0: it's inside the held item
	{
		ArcDots->AddInstance(FTransform(FRotator::ZeroRotator, Result.PathData[i].Location, FVector(DotScale)), true);
	}
	if (Result.HitResult.bBlockingHit)
	{
		ArcDots->AddInstance(FTransform(FRotator::ZeroRotator, Result.HitResult.ImpactPoint, FVector(DotScale * 2.5f)), true);
	}
}

void UKRThrowComponent::ClearArc()
{
	if (ArcDots)
	{
		ArcDots->ClearInstances();
	}
}
