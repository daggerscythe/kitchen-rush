// Copyright Epic Games, Inc. All Rights Reserved.


#include "KitchenRushPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "KitchenRushCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "UI/KRHUDWidget.h"
#include "KitchenRush.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Flow/KRActDirector.h"
#include "Flow/KRProfileSubsystem.h"
#include "UI/KRMainMenuWidget.h"
#include "UI/KRPauseMenuWidget.h"
#include "UI/KRSettingsWidget.h"
#include "UI/KRResultsWidget.h"
#include "UI/KRNoteCardWidget.h"

AKitchenRushPlayerController::AKitchenRushPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AKitchenRushCameraManager::StaticClass();
}

void AKitchenRushPlayerController::BeginPlay()
{
	Super::BeginPlay();

	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogKitchenRush, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	// Spawn the HUD
	if (IsLocalPlayerController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UKRHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToPlayerScreen(0);
		}
	}
}

void AKitchenRushPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		// [M4] Pause lives on the controller (the pawn's input stops while paused).
		if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (PauseAction)
			{
				EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AKitchenRushPlayerController::OnPausePressed);
			}
		}
	}
	
}

// ============================================================================
// [M4] Screens
// ============================================================================

void AKitchenRushPlayerController::OpenScreen(UUserWidget* Screen, int32 ZOrder)
{
	if (!Screen)
	{
		return;
	}
	if (!Screen->IsInViewport())
	{
		Screen->AddToPlayerScreen(ZOrder);
	}
	OpenScreens.AddUnique(Screen);

	SetPause(true);
	bShowMouseCursor = true;
	// Game AND UI: the buttons work, and the pause key still reaches us to close the pause menu.
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(Screen->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
}

void AKitchenRushPlayerController::CloseScreen(UUserWidget* Screen)
{
	if (Screen)
	{
		Screen->RemoveFromParent();
		OpenScreens.Remove(Screen);
	}
	if (OpenScreens.Num() > 0)
	{
		// Something is still up (e.g. back from Settings to the pause menu): focus it.
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(OpenScreens.Last()->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
		return;
	}
	// Back to cooking.
	SetPause(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

bool AKitchenRushPlayerController::IsAnyScreenOpen() const
{
	return OpenScreens.Num() > 0;
}

void AKitchenRushPlayerController::ShowMainMenu()
{
	if (!MainMenuClass)
	{
		// No menu set up yet: just start the night so testing still works.
		if (AKRActDirector* Director = AKRActDirector::Find(this))
		{
			Director->StartNight(1);
		}
		return;
	}
	if (!MainMenu)
	{
		MainMenu = CreateWidget<UKRMainMenuWidget>(this, MainMenuClass);
	}
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Collapsed); // the menu sits over the bare frozen kitchen
	}
	OpenScreen(MainMenu, 20);
}

void AKitchenRushPlayerController::PlayFromMenu()
{
	CloseScreen(MainMenu);
	if (AKRActDirector* Director = AKRActDirector::Find(this))
	{
		Director->StartNight(1);
	}
}

void AKitchenRushPlayerController::ContinueFromMenu()
{
	const UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this);
	RestartAtAct(Profile ? FMath::Max(Profile->GetLastActReached(), 1) : 1);
}

void AKitchenRushPlayerController::OnPausePressed()
{
	TogglePause();
}

void AKitchenRushPlayerController::TogglePause()
{
	if (PauseMenu && OpenScreens.Contains(PauseMenu))
	{
		if (Settings && OpenScreens.Contains(Settings))
		{
			CloseSettings(); // Esc from Settings goes back to the pause menu
			return;
		}
		CloseScreen(PauseMenu);
		return;
	}
	if (IsAnyScreenOpen() || !PauseMenuClass)
	{
		return; // main menu, a note or the results are up: pause doesn't apply
	}
	if (!PauseMenu)
	{
		PauseMenu = CreateWidget<UKRPauseMenuWidget>(this, PauseMenuClass);
	}
	OpenScreen(PauseMenu, 30);
}

void AKitchenRushPlayerController::OpenSettings()
{
	if (!SettingsClass)
	{
		return;
	}
	if (!Settings)
	{
		Settings = CreateWidget<UKRSettingsWidget>(this, SettingsClass);
	}
	OpenScreen(Settings, 40);
}

void AKitchenRushPlayerController::CloseSettings()
{
	CloseScreen(Settings);
}

void AKitchenRushPlayerController::ShowNote(const FText& Title, const FText& Body)
{
	NoteQueue.Add({ Title, Body });
	if (!NoteCard || !OpenScreens.Contains(NoteCard))
	{
		ShowNextNote();
	}
}

void AKitchenRushPlayerController::ShowNextNote()
{
	if (NoteQueue.Num() == 0 || !NoteCardClass)
	{
		NoteQueue.Empty();
		return;
	}
	const FQueuedNote Next = NoteQueue[0];
	NoteQueue.RemoveAt(0);
	if (!NoteCard)
	{
		NoteCard = CreateWidget<UKRNoteCardWidget>(this, NoteCardClass);
	}
	NoteCard->Setup(Next.Title, Next.Body);
	OpenScreen(NoteCard, 25);
}

void AKitchenRushPlayerController::CloseNote(bool bSkipTutorial)
{
	if (bSkipTutorial)
	{
		NoteQueue.Empty(); // every waiting note goes too
	}
	CloseScreen(NoteCard);
	if (bSkipTutorial)
	{
		if (AKRActDirector* Director = AKRActDirector::Find(this))
		{
			Director->SkipTutorial();
		}
	}
	else
	{
		ShowNextNote();
	}
}

void AKitchenRushPlayerController::ShowResults(const FKRResults& Results)
{
	NoteQueue.Empty();
	// Close whatever's up (a note, the pause menu), then show the results on their own.
	while (OpenScreens.Num() > 0)
	{
		UUserWidget* Top = OpenScreens.Last();
		Top->RemoveFromParent();
		OpenScreens.Pop();
	}
	if (!ResultsClass)
	{
		return;
	}
	if (!ResultsScreen)
	{
		ResultsScreen = CreateWidget<UKRResultsWidget>(this, ResultsClass);
	}
	ResultsScreen->Setup(Results);
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	OpenScreen(ResultsScreen, 50);
}

void AKitchenRushPlayerController::RestartAtAct(int32 Act)
{
	// Reload this level with an option the act director reads: "Act=2".
	const FString Level = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*Level), true, FString::Printf(TEXT("Act=%d"), FMath::Clamp(Act, 1, 3)));
}

void AKitchenRushPlayerController::RestartCurrentAct()
{
	const AKRActDirector* Director = AKRActDirector::Find(this);
	RestartAtAct(Director ? FMath::Max(Director->GetCurrentAct(), 1) : 1);
}

void AKitchenRushPlayerController::GoToMainMenu()
{
	// No options: the director shows the main menu.
	const FString Level = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*Level), true);
}

void AKitchenRushPlayerController::QuitToDesktop()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

bool AKitchenRushPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
