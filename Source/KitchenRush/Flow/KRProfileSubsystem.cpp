// KRProfileSubsystem.cpp

#include "Flow/KRProfileSubsystem.h"
#include "Flow/KRSaveGame.h"
#include "Time/KRTimeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

static const TCHAR* GSlotName = TEXT("KitchenRush");

void UKRProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameplayStatics::DoesSaveGameExist(GSlotName, 0))
	{
		Data = Cast<UKRSaveGame>(UGameplayStatics::LoadGameFromSlot(GSlotName, 0));
	}
	if (!Data)
	{
		Data = Cast<UKRSaveGame>(UGameplayStatics::CreateSaveGameObject(UKRSaveGame::StaticClass()));
	}

	// First run: start from Project Settings (time modes) and the current Application Scale (UI size).
	if (!Data->bSettingsInitialised)
	{
		const UKRTimeSettings* TimeSettings = GetDefault<UKRTimeSettings>();
		Data->StillMode = TimeSettings->StillMode;
		Data->MoveMode = TimeSettings->MoveMode;
		Data->UIScale = GetDefault<UUserInterfaceSettings>()->ApplicationScale;
		Data->MouseSensitivity = 1.f;
		Data->bSettingsInitialised = true;
		Save();
	}
}

UKRProfileSubsystem* UKRProfileSubsystem::Get(const UObject* WorldContext)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
	return GameInstance ? GameInstance->GetSubsystem<UKRProfileSubsystem>() : nullptr;
}

void UKRProfileSubsystem::Save()
{
	UGameplayStatics::SaveGameToSlot(Data, GSlotName, 0);
}

int32 UKRProfileSubsystem::GetHighScore() const { return Data->HighScore; }
int32 UKRProfileSubsystem::GetLastActReached() const { return Data->LastActReached; }
float UKRProfileSubsystem::GetMouseSensitivity() const { return Data->MouseSensitivity; }
float UKRProfileSubsystem::GetUIScale() const { return Data->UIScale; }
EKRStillMode UKRProfileSubsystem::GetStillMode() const { return Data->StillMode; }
EKRMoveMode UKRProfileSubsystem::GetMoveMode() const { return Data->MoveMode; }

void UKRProfileSubsystem::RecordAct(int32 Act)
{
	if (Act > Data->LastActReached)
	{
		Data->LastActReached = FMath::Clamp(Act, 1, 3);
		Save();
	}
}

bool UKRProfileSubsystem::RecordTips(int32 Tips, bool bWon)
{
	const bool bNewBest = Tips > Data->HighScore;
	if (bNewBest)
	{
		Data->HighScore = Tips;
	}
	Data->bHasWon |= bWon;
	Save();
	return bNewBest;
}

void UKRProfileSubsystem::SetMouseSensitivity(float Value)
{
	Data->MouseSensitivity = FMath::Clamp(Value, 0.1f, 5.f);
	Save();
}

void UKRProfileSubsystem::SetUIScale(float Value)
{
	Data->UIScale = FMath::Clamp(Value, 0.5f, 3.f);
	GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = Data->UIScale;
	Save();
}

void UKRProfileSubsystem::SetStillMode(EKRStillMode Mode, UWorld* World)
{
	Data->StillMode = Mode;
	Save();
	ApplySettings(World);
}

void UKRProfileSubsystem::SetMoveMode(EKRMoveMode Mode, UWorld* World)
{
	Data->MoveMode = Mode;
	Save();
	ApplySettings(World);
}

void UKRProfileSubsystem::ApplySettings(UWorld* World) const
{
	// UI scale is an engine-wide setting (the same one as Project Settings > User Interface > Application Scale).
	GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = Data->UIScale;

	if (UKRTimeSubsystem* Time = World ? World->GetSubsystem<UKRTimeSubsystem>() : nullptr)
	{
		Time->SetStillMode(Data->StillMode);
		Time->SetMoveMode(Data->MoveMode);
	}
}
