// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Misc/EnumRange.h"
#include "DialogueDataStructs.generated.h"

UENUM()
enum class EDialogueState : uint8 {
  None UMETA(DisplayName = "NONE"),
  Dialogue UMETA(DisplayName = "Dialogue"),
  PlayerChoice UMETA(DisplayName = "Player Choice"),
  PlayerInquire UMETA(DisplayName = "Player Inquire"),
  EndPlayerInquire UMETA(DisplayName = "End Player Inquire"),
  End UMETA(DisplayName = "End"),
};
UENUM()
enum class EDialogueAction : uint8 {
  None UMETA(DisplayName = "NONE"),
  NextDialogue UMETA(DisplayName = "Dialogue Next"),
  StartPlayerChoice UMETA(DisplayName = "Start Player Choice"),
  StartPlayerInquire UMETA(DisplayName = "Start Player Inquire"),
  EndPlayerInquire UMETA(DisplayName = "End Player Inquire"),  // Ends the whole inquire block.
  End UMETA(DisplayName = "End"),  // Ends the dialogue, ending the whole chain or returning to the inquire block.
};
UENUM()
enum class EDialogueSpeaker : uint8 {
  None UMETA(DisplayName = "NONE"),
  NPC UMETA(DisplayName = "NPC"),
  Player UMETA(DisplayName = "Player"),
};
UENUM()
enum class EDialogueType : uint8 {
  Dialogue UMETA(DisplayName = "Dialogue"),
  Choice UMETA(DisplayName = "Choice"),
  Inquire UMETA(DisplayName = "Inquire"),
  Pointer UMETA(DisplayName = "Pointer"),  // Consumes the pointed node to return the children.
};

UENUM()
enum class EGameEffectType : uint8 {
  None UMETA(DisplayName = "NONE"),
  AddPlayerTag UMETA(DisplayName = "Add Player Tag"),
  RemovePlayerTag UMETA(DisplayName = "Remove Player Tag"),
  PlaySound UMETA(DisplayName = "Play Sound"),
  PlayMusic UMETA(DisplayName = "Play Music"),
  Script UMETA(DisplayName = "Script"),  // VordieScript.
};
USTRUCT()
struct FDialogueEffect {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, SaveGame)
  EGameEffectType GameEffectType;
  UPROPERTY(EditAnywhere, SaveGame)
  FName RelevantID;
};

USTRUCT()
struct FDialogueData {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, SaveGame)
  FName DialogueChainID;
  UPROPERTY(EditAnywhere, SaveGame)
  FName DialogueID;
  UPROPERTY(EditAnywhere, SaveGame)
  EDialogueType DialogueType;

  UPROPERTY(EditAnywhere, SaveGame)
  FText DialogueText;
  UPROPERTY(EditAnywhere, SaveGame)
  EDialogueAction Action;

  UPROPERTY(EditAnywhere, SaveGame)
  EDialogueSpeaker DialogueSpeaker;
  UPROPERTY(EditAnywhere, SaveGame)
  FText SpeakerName;  // * Override

  UPROPERTY(EditAnywhere, SaveGame)
  int32 BranchChildrenAmount;
  UPROPERTY(EditAnywhere, SaveGame)
  FName PointerID;  // * For pointer type dialogues.

  UPROPERTY(EditAnywhere, SaveGame)
  FGameplayTagContainer DialogueTags;
};
USTRUCT()
struct FDialogueDataTable : public FTableRowBase {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere)
  FName DialogueChainID;
  UPROPERTY(EditAnywhere)
  FName DialogueID;
  UPROPERTY(EditAnywhere)
  EDialogueType DialogueType;

  UPROPERTY(EditAnywhere)
  FText DialogueText;
  UPROPERTY(EditAnywhere)
  EDialogueAction Action;

  UPROPERTY(EditAnywhere)
  EDialogueSpeaker DialogueSpeaker;
  UPROPERTY(EditAnywhere, SaveGame)
  FText SpeakerName;  // * Override

  UPROPERTY(EditAnywhere)
  int32 BranchChildrenAmount;  // Note: To know number of children in a preorder tree traversal.
  UPROPERTY(EditAnywhere)
  int32 ChoiceIndex;  // Note: This is really just for nicer understanding in the data table.

  UPROPERTY(EditAnywhere)
  FGameplayTagContainer DialogueTags;
};

USTRUCT()
struct FNextDialogueRes {
  GENERATED_BODY()

  TOptional<FDialogueData> DialogueData;
  EDialogueState State;
};

// * This enables using a list in a map.
USTRUCT()
struct FDialoguesArray {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere)
  TArray<struct FDialogueData> Dialogues;
};