// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Prototyping/Dialogue/DialogueDataStructs.h"
#include "InteractionComponent.generated.h"

// ? Add start interaction and end interaction events/callbacks.

UENUM()
enum class EInteractionType : uint8 {
  None UMETA(DisplayName = "None"),

  Use UMETA(DisplayName = "Use"),
  Examine UMETA(DisplayName = "Examine"),
  Dialogue UMETA(DisplayName = "Dialogue"),
  Pickup UMETA(DisplayName = "Pickup"),

  LevelChange UMETA(DisplayName = "Level Change"),

  Container UMETA(DisplayName = "Container"),
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPING_API UInteractionComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UInteractionComponent();

  virtual void BeginPlay() override;

  UPROPERTY(EditAnywhere, SaveGame)
  EInteractionType InteractionType;

  UPROPERTY(EditAnywhere)
  class USoundBase* InteractionSound;

  void PlayInteractionSound() const;

  UPROPERTY(EditAnywhere)
  bool bIsInteracting;  // * For interactions that need tracking.
  void StartInteraction();
  void EndInteraction();

  void InteractUse() const;
  void InteractExamine() const;
  auto InteractDialogue() const -> TOptional<class UDialogueComponent*>;
  // auto InteractPickup() const -> TOptional<class UPickupComponent*>;

  // auto InteractLevelChange() const -> class ULevelChangeComponent*;

  // auto InteractContainer() const -> class UInventoryComponent*;
};
