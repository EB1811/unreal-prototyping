// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InGameGameMode.generated.h"

UCLASS()
class PROTOTYPING_API AInGameGameMode : public AGameModeBase {
  GENERATED_BODY()

public:
  AInGameGameMode();

  virtual void BeginPlay() override;

  UPROPERTY()
  class APlayerCharacter* PlayerCharacter;
  UPROPERTY()
  class AInGameControlHUD* ControlHUD;
};
