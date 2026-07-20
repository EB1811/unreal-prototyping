// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/WidgetAnimation.h"
#include "CoreMinimal.h"
#include "UIStructs.generated.h"

// These are meant to be used as components of other classes, rather than as standalone structs themselves.
// Using unreal's reflection system to retrieve the functions and properties of these structs at runtime.
// Could instead use interfaces, but that would require more boilerplate code and would be less flexible.

// * Common UI actions.
USTRUCT()
struct FUIActionable {
  GENERATED_BODY()

  TFunction<void()> AdvanceUI;
  TFunction<void()> AdvanceUIHold;
  TFunction<void()> RetractUI;
  TFunction<void()> QuitUI;

  TFunction<void(float)> NumericInput;
  TFunction<void(FVector2D)> DirectionalInput;

  // * Side buttons for various actions, e.g., sorting, filtering, etc.
  TFunction<void()> SideButton1;
  TFunction<void()> SideButton2;
  TFunction<void()> SideButton3;
  TFunction<void()> SideButton4;

  // * Cycle buttons.
  TFunction<void()> CycleLeft;
  TFunction<void()> CycleRight;
};

// * UI Flavor like animations and sounds.
USTRUCT()
struct FUIBehaviour {
  GENERATED_BODY()

  // * Attempt to pass the animnation itself.
  UWidgetAnimation* ShowAnim;
  UWidgetAnimation* HideAnim;
  USoundBase* OpenSound;
  USoundBase* HideSound;
};

// * UI refresh behavior, e.g., for dynamic content.
USTRUCT()
struct FUIRefresh {
  GENERATED_BODY()

  // ! Not implemented.
  float RefreshInterval;  // Optional custom refresh interval, if 0, will use default interval.

  TFunction<void()> RefreshTick;
};