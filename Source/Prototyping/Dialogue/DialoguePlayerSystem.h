// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueDataStructs.h"
#include "DialoguePlayerSystem.generated.h"

UCLASS(Blueprintable)
class PROTOTYPING_API UDialoguePlayerSystem : public UObject {
  GENERATED_BODY()

public:
  UDialoguePlayerSystem() : DialogueState(EDialogueState::None), CurrDialogueIndex(0) {}

  UPROPERTY(EditAnywhere)
  EDialogueState DialogueState;

  UPROPERTY(EditAnywhere)
  class UDialogueComponent* DialogueC;
  UPROPERTY(EditAnywhere)
  FText SpeakerName;
  UPROPERTY(EditAnywhere)
  TArray<FDialogueData> DialogueDataArr;

  TFunction<void()> DialogueClosedFunc;
  TFunction<void()> DialogueFinishedFunc;

  UPROPERTY(EditAnywhere)
  int32 CurrDialogueIndex;
  UPROPERTY(EditAnywhere)
  TArray<int32> InquireBlockIndexes;

  void StartDialogue();
  void StartDialogue(const TArray<FDialogueData> _DialogueDataArr,
                     const FString& _SpeakerName = "NPC",
                     TFunction<void()> _DialogueClosedFunc = nullptr,
                     TFunction<void()> _DialogueFinishedFunc = nullptr);
  void StartDialogue(class UDialogueComponent* _DialogueC,
                     TFunction<void()> _DialogueClosedFunc = nullptr,
                     TFunction<void()> _DialogueFinishedFunc = nullptr);

  void NextDialogue();

  auto GetChoiceDialogues() -> TArray<FDialogueData>;
  void DialogueChoice(int32 ChoiceIndex);
  auto GetInquireDialogues() -> TArray<FDialogueData>;
  void InquireDialogue(int32 InquireIndex);

  void CloseDialogue();
  void FinishDialogue();

  void ResetDialogue();
};