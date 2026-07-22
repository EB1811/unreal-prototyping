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
  UGameStateSubsystem() { CurrentGameState = GlobalGameState::InGame; }

  UPROPERTY(EditAnywhere)
  GlobalGameState CurrentGameState;

  FGameStateChangedDelegate GameStateChangedDelegate;

  void ChangeGameState(GlobalGameState NewGameState);
};