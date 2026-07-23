// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueDataStructs.h"
#include "DialoguePlayerSystem.generated.h"

EDialogueState GetNextDialogueState(EDialogueState CurrentState, EDialogueAction Action);
TArray<int32> GetChildChoiceIndexes(const TArray<FDialogueData>& DialogueDataArr,
                                    int32 StartIndex,
                                    int32 ChoicesAmount);

UCLASS(Blueprintable)
class PROTOTYPING_API UDialoguePlayerSystem : public UObject {
  GENERATED_BODY()

public:
  UDialoguePlayerSystem() : DialogueState(EDialogueState::None), CurrentDialogueIndex(0) {}

  UPROPERTY(EditAnywhere)
  class UDialogueComponent* DialogueC;

  UPROPERTY(EditAnywhere)
  EDialogueState DialogueState;

  UPROPERTY(EditAnywhere)
  FText SpeakerName;
  UPROPERTY(EditAnywhere)
  TArray<FDialogueData> DialogueDataArr;
  UPROPERTY(EditAnywhere)
  int32 CurrentDialogueIndex;
  UPROPERTY(EditAnywhere)
  TArray<int32> InquireBlockIndexes;

  UPROPERTY(EditAnywhere)
  TArray<FName> ChoiceDialoguesSelectedIDs;

  FNextDialogueRes StartDialogue(class UDialogueComponent* _DialogueC);
  FNextDialogueRes StartDialogue(const TArray<FDialogueData> _DialogueDataArr, const FString& _SpeakerName = "NPC");

  void NextDialogue();
  TArray<FDialogueData> GetChoiceDialogues();
  FNextDialogueRes DialogueChoice(int32 ChoiceIndex);
  TArray<FDialogueData> GetInquireDialogues();
  FNextDialogueRes InquireDialogue(int32 InquireIndex);

  void ResetDialogue();
};