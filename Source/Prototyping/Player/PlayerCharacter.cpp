
#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Prototyping/Framework/Subsystems/GameStateSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacter::APlayerCharacter() {}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
    // In Game
    EnhancedInputComponent->BindAction(InGameInputActions.OpenPauseMenuAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::OpenPauseMenu);
    // In UI
    EnhancedInputComponent->BindAction(InUIInputActions.AdvanceUIAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::AdvanceUI);
    EnhancedInputComponent->BindAction(InUIInputActions.UIDirectionalInputAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UIDirectionalInputAction);
  }
}

void APlayerCharacter::BeginPlay() {
  Super::BeginPlay();

  ControlHUD = Cast<AInGameControlHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

  GameStateSubsystem = GetSubsystem<UGameStateSubsystem>(GetWorld());
  GameStateSubsystem->GameStateChangedDelegate.AddUObject(this, &APlayerCharacter::HandleGlobalGameStateChanged);

  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  Subsystem->AddMappingContext(InputContexts[GameStateSubsystem->CurrentGameState], 0);
}

void APlayerCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void APlayerCharacter::HandleGlobalGameStateChanged(GlobalGameState OldGameState, GlobalGameState NewGameState) {
  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  Subsystem->RemoveMappingContext(InputContexts[OldGameState]);
  Subsystem->AddMappingContext(InputContexts[NewGameState], 0);
}

void APlayerCharacter::OpenPauseMenu(const FInputActionValue& Value) {
  if (!ControlHUD) return;

  ControlHUD->OpenPauseMenuView();
}
void APlayerCharacter::AdvanceUI(const FInputActionValue& Value) {
  if (!ControlHUD) return;

  ControlHUD->AdvanceUIAction();
}
void APlayerCharacter::UIDirectionalInputAction(const FInputActionValue& Value) {
  if (!ControlHUD) return;

  ControlHUD->UIDirectionalInputAction(Value.Get<FVector2D>());
}