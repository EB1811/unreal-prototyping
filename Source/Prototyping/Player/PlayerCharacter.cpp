
#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacter::APlayerCharacter() {}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
    EnhancedInputComponent->BindAction(InGameInputActions.OpenPauseMenuAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::OpenPauseMenu);
  }
}

void APlayerCharacter::BeginPlay() {
  Super::BeginPlay();

  ControlHUD = Cast<AInGameControlHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  Subsystem->AddMappingContext(InputContext, 0);
}

void APlayerCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void APlayerCharacter::OpenPauseMenu(const FInputActionValue& Value) {
  UE_LOG(LogTemp, Log, TEXT("OpenPauseMenu triggered"));

  if (!ControlHUD) return;

  ControlHUD->OpenPauseMenuView();
}