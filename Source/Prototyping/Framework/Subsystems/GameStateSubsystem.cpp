#include "GameStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UGameStateSubsystem::ChangeGameState(GlobalGameState NewGameState) {
  if (CurrGameState == NewGameState) return;

  if (NewGameState == GlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), true);
  if (CurrGameState == GlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), false);

  PrevGameState = CurrGameState;
  CurrGameState = NewGameState;
  GameStateChangedDelegate.Broadcast(PrevGameState, NewGameState);
}