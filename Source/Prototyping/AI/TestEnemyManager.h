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

USTRUCT(BlueprintType)
struct FTestEnemyStats {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere)
  int32 Strength;
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
  TMap<FString, int32> StatsMap;

  UPROPERTY(EditAnywhere)
  FTestEnemyStats EnemyStats;

  // CallInEditor is required so AActor::ProcessEvent doesn't silently skip the call when the actor
  // isn't in an initialized game world (e.g. NewObject'd directly in an automation test).
  UFUNCTION(BlueprintCallable, CallInEditor)
  void MaxHealth() { Health = 100; };

  UFUNCTION(BlueprintCallable, CallInEditor)
  void RemoveHealth(int32 Damage) { Health -= Damage; };

  UFUNCTION(BlueprintCallable, CallInEditor)
  int32 GetHealth() { return Health; };

  UFUNCTION(BlueprintCallable, CallInEditor)
  FTestEnemyStats GetStats() { return EnemyStats; };
};