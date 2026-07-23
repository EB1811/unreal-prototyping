#include "GameStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UGameStateSubsystem::ChangeGameState(EGlobalGameState NewGameState) {
  if (CurrGameState == NewGameState) return;

  if (NewGameState == EGlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), true);
  if (CurrGameState == EGlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), false);

  PrevGameState = CurrGameState;
  CurrGameState = NewGameState;
  GameStateChangedDelegate.Broadcast(PrevGameState, NewGameState);
}