#include "GameStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UGameStateSubsystem::ChangeGameState(EGlobalGameState NewGameState) {
  if (CurrGameState == NewGameState) return;

  UE_LOG(LogTemp, Log, TEXT("Changing Game State from %s to %s"), *UEnum::GetValueAsString(CurrGameState),
         *UEnum::GetValueAsString(NewGameState));

  if (CurrGameState == EGlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), false);
  if (NewGameState == EGlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), true);

  PrevGameState = CurrGameState;
  CurrGameState = NewGameState;
  GameStateChangedDelegate.Broadcast(PrevGameState, CurrGameState);
}