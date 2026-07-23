// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

UENUM()
enum class EDialogueComponentType : uint8 {
  Simple UMETA(DisplayName = "Simple"),    // Single dialogue chain.
  Rolling UMETA(DisplayName = "Rolling"),  // Many dialogue chains, next one each interaction.
  Random UMETA(DisplayName = "Random"),    // Random dialogue chain.
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPING_API UDialogueComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UDialogueComponent() {
    PrimaryComponentTick.bCanEverTick = false;
    DialogueComponentType = EDialogueComponentType::Simple;
  }

  virtual void BeginPlay() override;

  UPROPERTY(EditAnywhere, Category = "Dialogue", SaveGame)
  EDialogueComponentType DialogueComponentType;

  UPROPERTY(EditAnywhere, Category = "Dialogue", SaveGame)
  FText SpeakerName;
  UPROPERTY(EditAnywhere, Category = "Dialogue", SaveGame)
  TArray<struct FDialogueData> DialogueArray;
  UPROPERTY(EditAnywhere, Category = "Dialogue", SaveGame)
  FName CurrentDialogueChainId;

  auto GetNextDialogueChain() -> TArray<struct FDialogueData>;
  void FinishReadingDialogueChain();
};