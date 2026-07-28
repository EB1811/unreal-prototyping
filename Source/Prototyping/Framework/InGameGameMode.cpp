
#include "InGameGameMode.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/Dialogue/DialogueSystem.h"
#include "Prototyping/Player/PlayerCharacter.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Prototyping/AI/TestEnemyManager.h"

AInGameGameMode::AInGameGameMode() {}

void AInGameGameMode::BeginPlay() {
  Super::BeginPlay();

  UE_LOG(LogTemp, Log, TEXT("Initializing Systems."));

  PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
  ControlHUD = Cast<AInGameControlHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
  check(PlayerCharacter && ControlHUD);
  check(DialogueSystemClass && TestEnemyManagerClass);

  DialogueSystem = GetWorld()->SpawnActor<ADialogueSystem>(DialogueSystemClass);

  TestEnemyManager = GetWorld()->SpawnActor<ATestEnemyManager>(TestEnemyManagerClass);

  PlayerCharacter->DialogueSystem = DialogueSystem;

  ControlHUD->DialogueSystem = DialogueSystem;
  UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
  ControlHUDSubsystem->RegisterHUD(ControlHUD);

  TestEnemyManager->PlayerCharacter = PlayerCharacter;
  TestEnemyManager->DialogueSystem = DialogueSystem;

  ControlHUD->InitUIWidgets();

  // Settings
  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  check(Subsystem);
  UEnhancedInputUserSettings* EISettings = Subsystem->GetUserSettings();
  check(EISettings);
  for (const auto& StateContext : PlayerCharacter->InputContexts)
    EISettings->RegisterInputMappingContext(StateContext.Value);
}