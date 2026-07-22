// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Prototyping/Framework/GameStateStructs.h"
#include "GameStateSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FGameStateChangedDelegate,
                                     GlobalGameState,
                                     GlobalGameState);  // * Old state, new state.

UCLASS()
class PROTOTYPING_API UGameStateSubsystem : public UWorldSubsystem {
  GENERATED_BODY()

public:
  UGameStateSubsystem() { CurrGameState = GlobalGameState::InGame; }

  FGameStateChangedDelegate GameStateChangedDelegate;

  UPROPERTY(EditAnywhere)
  GlobalGameState PrevGameState;
  UPROPERTY(EditAnywhere)
  GlobalGameState CurrGameState;

  void ChangeGameState(GlobalGameState NewGameState);
};