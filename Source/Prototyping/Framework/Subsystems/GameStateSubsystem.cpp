#include "GameStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UGameStateSubsystem::ChangeGameState(GlobalGameState NewGameState) {
  if (CurrentGameState == NewGameState) return;

  if (NewGameState == GlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), true);
  if (CurrentGameState == GlobalGameState::Paused) UGameplayStatics::SetGamePaused(GetWorld(), false);

  GlobalGameState OldGameState = CurrentGameState;
  CurrentGameState = NewGameState;
  GameStateChangedDelegate.Broadcast(OldGameState, NewGameState);
}