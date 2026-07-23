// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameStateStructs.generated.h"

UENUM()
enum class EGlobalGameState : uint8 {
  InGame UMETA(DisplayName = "InGame"),
  FocussedMenu UMETA(DisplayName = "FocusedMenu"),  // * A menu is open and focused.
  NoControl UMETA(DisplayName = "NoControl"),       // * No input control (e.g., during loading).
  Paused UMETA(DisplayName = "Paused"),
  GameOver UMETA(DisplayName = "GameOver"),
};