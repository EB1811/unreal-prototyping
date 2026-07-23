#include "DialoguePlayerSystem.h"
#include "DialogueDataStructs.h"
#include "DialogueComponent.h"
#include "HAL/Platform.h"

EDialogueState GetNextDialogueState(EDialogueState CurrentState, EDialogueAction Action) {
  switch (Action) {
    case EDialogueAction::NPCNext: return EDialogueState::NPCTalk;
    case EDialogueAction::PlayerNext: return EDialogueState::PlayerTalk;
    case EDialogueAction::AskPlayer: return EDialogueState::PlayerChoice;
    case EDialogueAction::StartPlayerInquire: return EDialogueState::PlayerInquire;
    case EDialogueAction::EndPlayerInquire: return EDialogueState::EndPlayerInquire;
    case EDialogueAction::End: return EDialogueState::End;
    default: return EDialogueState::None;
  }
}

// Note: Preorder traversal, only looking for direct children.
TArray<int32> GetChildChoiceIndexes(const TArray<FDialogueData>& DialogueDataArr,
                                    int32 StartIndex,
                                    int32 ChoicesAmount) {
  TArray<int32> ChoiceDialogueIndexes;
  ChoiceDialogueIndexes.Reserve(ChoicesAmount);

  int32 FoundChoices = 0;
  int32 IgnoreNext = 0;
  for (int32 i = StartIndex; i < DialogueDataArr.Num(); i += 1) {
    if (DialogueDataArr[i].DialogueType == EDialogueType::Choice) {
      if (IgnoreNext > 0) {
        IgnoreNext -= 1;
        continue;
      }

      ChoiceDialogueIndexes.Add(i);
      if (ChoiceDialogueIndexes.Num() == ChoicesAmount) break;
    }
    if (DialogueDataArr[i].Action == EDialogueAction::AskPlayer) IgnoreNext = DialogueDataArr[i].ChoicesAmount;
  }

  return ChoiceDialogueIndexes;
}
inline auto GetChildInquireIndexes(const TArray<FDialogueData>& DialogueDataArr, int32 StartIndex) -> TArray<int32> {
  int32 InquireAmount = DialogueDataArr[StartIndex].ChoicesAmount;
  TArray<int32> DialogueIndexes;
  DialogueIndexes.Reserve(InquireAmount);

  int32 FoundInquire = 0;
  int32 IgnoreNext = 0;
  for (int32 i = StartIndex + 1; i < DialogueDataArr.Num(); i += 1) {
    if (DialogueDataArr[i].DialogueType == EDialogueType::Inquire) {
      if (IgnoreNext > 0) {
        IgnoreNext -= 1;
        continue;
      }

      DialogueIndexes.Add(i);
      if (DialogueIndexes.Num() == InquireAmount) break;
    }
    if (DialogueDataArr[i].Action == EDialogueAction::StartPlayerInquire) IgnoreNext = DialogueDataArr[i].ChoicesAmount;
  }

  return DialogueIndexes;
}

FNextDialogueRes UDialoguePlayerSystem::StartDialogue(class UDialogueComponent* _DialogueC) {
  check(_DialogueC && _DialogueC->DialogueArray.Num() > 0);

  ResetDialogue();

  DialogueC = _DialogueC;
  DialogueState =
      GetNextDialogueState(DialogueState, DialogueC->DialogueArray[0].DialogueSpeaker == EDialogueSpeaker::NPC
                                              ? EDialogueAction::NPCNext
                                              : EDialogueAction::PlayerNext);
  SpeakerName = DialogueC->SpeakerName;
  DialogueDataArr = DialogueC->GetNextDialogueChain();
  CurrentDialogueIndex = 0;
  ChoiceDialoguesSelectedIDs.Empty();

  return {DialogueDataArr[CurrentDialogueIndex], DialogueState};
}

FNextDialogueRes UDialoguePlayerSystem::StartDialogue(const TArray<FDialogueData> _DialogueDataArr,
                                                      const FString& _SpeakerName) {
  check(_DialogueDataArr.Num() > 0);

  ResetDialogue();

  DialogueState = GetNextDialogueState(DialogueState, _DialogueDataArr[0].DialogueSpeaker == EDialogueSpeaker::NPC
                                                          ? EDialogueAction::NPCNext
                                                          : EDialogueAction::PlayerNext);
  SpeakerName = FText::FromString(_SpeakerName);
  DialogueDataArr = _DialogueDataArr;
  CurrentDialogueIndex = 0;
  ChoiceDialoguesSelectedIDs.Empty();

  return {DialogueDataArr[CurrentDialogueIndex], DialogueState};
}

void UDialoguePlayerSystem::NextDialogue() {
  DialogueState = GetNextDialogueState(DialogueState, DialogueDataArr[CurrentDialogueIndex].Action);

  switch (DialogueState) {
    case EDialogueState::NPCTalk:
    case EDialogueState::PlayerTalk: {
      CurrentDialogueIndex++;
      break;
    }
    case EDialogueState::PlayerChoice: {
      break;
    }
    // todo-low: Handle this through actions.
    case EDialogueState::PlayerInquire: {
      InquireBlockIndexes.Add(CurrentDialogueIndex);

      break;
    }
    case EDialogueState::EndPlayerInquire: {
      check(InquireBlockIndexes.Num() > 0);
      InquireBlockIndexes.Pop();

      if (InquireBlockIndexes.Num() > 0) {
        CurrentDialogueIndex = InquireBlockIndexes.Last();
        DialogueState = EDialogueState::PlayerInquire;
        break;
      }

      // No more inquire blocks, end dialogue.
      DialogueState = EDialogueState::End;
      if (DialogueC) DialogueC->FinishReadingDialogueChain();
      break;
    }
    case EDialogueState::End: {
      if (InquireBlockIndexes.Num() > 0) {
        CurrentDialogueIndex = InquireBlockIndexes.Last();
        DialogueState = EDialogueState::PlayerInquire;
        break;
      }

      if (DialogueC) DialogueC->FinishReadingDialogueChain();
      break;
    }
    default: {
      checkf(false, TEXT("Invalid Dialogue State"));
      break;
    }
  }
}

TArray<FDialogueData> UDialoguePlayerSystem::GetChoiceDialogues() {
  if (DialogueState != EDialogueState::PlayerChoice) return {};

  const TArray<int32>& ChildChoiceIndexes = GetChildChoiceIndexes(DialogueDataArr, CurrentDialogueIndex + 1,
                                                                  DialogueDataArr[CurrentDialogueIndex].ChoicesAmount);
  TArray<FDialogueData> ChoiceDialogueArr = {};
  ChoiceDialogueArr.Reserve(ChildChoiceIndexes.Num());
  for (int32 ChoiceIndex : ChildChoiceIndexes) ChoiceDialogueArr.Add(DialogueDataArr[ChoiceIndex]);

  return ChoiceDialogueArr;
}
FNextDialogueRes UDialoguePlayerSystem::DialogueChoice(int32 ChoiceIndex) {
  check(ChoiceIndex >= 0 && ChoiceIndex < DialogueDataArr[CurrentDialogueIndex].ChoicesAmount);
  check(DialogueState == EDialogueState::PlayerChoice);

  const TArray<int32>& ChildChoiceIndexes = GetChildChoiceIndexes(DialogueDataArr, CurrentDialogueIndex + 1,
                                                                  DialogueDataArr[CurrentDialogueIndex].ChoicesAmount);
  int32 ChoiceDialogueIndex = ChildChoiceIndexes[ChoiceIndex];

  ChoiceDialoguesSelectedIDs.Add(DialogueDataArr[ChoiceDialogueIndex].DialogueID);

  CurrentDialogueIndex = ChoiceDialogueIndex;
  NextDialogue();
  return {DialogueDataArr[CurrentDialogueIndex], DialogueState};
}

TArray<FDialogueData> UDialoguePlayerSystem::GetInquireDialogues() {
  check(DialogueState == EDialogueState::PlayerInquire);

  const TArray<int32>& ChildInquireIndexes = GetChildInquireIndexes(DialogueDataArr, CurrentDialogueIndex);
  TArray<FDialogueData> InquireDialogueArr = {};
  InquireDialogueArr.Reserve(ChildInquireIndexes.Num());
  for (int32 InquireIndex : ChildInquireIndexes) InquireDialogueArr.Add(DialogueDataArr[InquireIndex]);

  return InquireDialogueArr;
}
FNextDialogueRes UDialoguePlayerSystem::InquireDialogue(int32 InquireIndex) {
  check(InquireIndex >= 0 && InquireIndex < DialogueDataArr[CurrentDialogueIndex].ChoicesAmount);
  check(DialogueState == EDialogueState::PlayerInquire);

  const TArray<int32>& ChildInquireIndexes = GetChildInquireIndexes(DialogueDataArr, CurrentDialogueIndex);
  CurrentDialogueIndex = ChildInquireIndexes[InquireIndex];
  NextDialogue();
  return {DialogueDataArr[CurrentDialogueIndex], DialogueState};
}

void UDialoguePlayerSystem::ResetDialogue() {
  DialogueC = nullptr;
  DialogueState = EDialogueState::None;
  DialogueDataArr.Empty();
  CurrentDialogueIndex = 0;
  InquireBlockIndexes.Empty();
  ChoiceDialoguesSelectedIDs.Empty();
}