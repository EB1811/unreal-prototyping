// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "TestEnemyManager.generated.h"

USTRUCT()
struct FTestEnemyBehaviorParams {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, SaveGame)
  bool bAggressive;

  UPROPERTY(EditAnywhere)
  TArray<int32> Levels;
};

UCLASS(Blueprintable)
class PROTOTYPING_API ATestEnemyManager : public AInfo {
  GENERATED_BODY()

public:
  ATestEnemyManager() {
    PrimaryActorTick.bCanEverTick = false;

    BehaviourParams.bAggressive = false;
  }

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditAnywhere)
  class APlayerCharacter* PlayerCharacter;
  UPROPERTY(EditAnywhere)
  class ADialogueSystem* DialogueSystem;

  UPROPERTY(EditAnywhere, SaveGame)
  struct FTestEnemyBehaviorParams BehaviourParams;

  UPROPERTY(EditAnywhere)
  int32 Health = 0;

  UPROPERTY(EditAnywhere)
  bool bIsAlive = false;

  UPROPERTY(EditAnywhere)
  FString Name;

  UPROPERTY(EditAnywhere)
  TMap<FString, int32> Stats;
};