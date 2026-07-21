
#include "InGameGameMode.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/Player/PlayerCharacter.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AInGameGameMode::AInGameGameMode() {}

void AInGameGameMode::BeginPlay() {
  Super::BeginPlay();

  UE_LOG(LogTemp, Log, TEXT("Initializing Systems."));

  PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

  ControlHUD = Cast<AInGameControlHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
  ControlHUD->InitUIWidgets();

  UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
  ControlHUDSubsystem->RegisterHUD(ControlHUD);

  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
}