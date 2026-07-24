#include "DialoguePlayerSystem.h"
#include "DialogueDataStructs.h"
#include "DialogueComponent.h"
#include "HAL/Platform.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/UI/InGameControlHUD.h"

inline auto GetNextDialogueState(EDialogueState CurrentState, EDialogueAction Action) -> EDialogueState {
  switch (Action) {
    case EDialogueAction::NextDialogue: return EDialogueState::Dialogue;
    case EDialogueAction::StartPlayerChoice: return EDialogueState::PlayerChoice;
    case EDialogueAction::StartPlayerInquire: return EDialogueState::PlayerInquire;
    case EDialogueAction::EndPlayerInquire: return EDialogueState::EndPlayerInquire;
    case EDialogueAction::End: return EDialogueState::End;
    case EDialogueAction::None: return CurrentState;
    default: return EDialogueState::None;
  }
}
// Note: Preorder traversal, only looking for direct children.
inline auto GetDialogueTreeChildIndexes(const TArray<FDialogueData>& DialogueDataArr,
                                        int32 StartIndex,
                                        EDialogueType ChildType,
                                        EDialogueAction BranchAction) -> TArray<int32> {
  int32 ChildrenAmount = DialogueDataArr[StartIndex].BranchChildrenAmount;
  TArray<int32> DialogueIndexes;
  DialogueIndexes.Reserve(ChildrenAmount);

  int32 IgnoreNext = 0;
  for (int32 i = StartIndex + 1; i < DialogueDataArr.Num(); i += 1) {
    if (DialogueDataArr[i].DialogueType == ChildType) {
      if (IgnoreNext > 0) {
        IgnoreNext -= 1;
        continue;
      }

      DialogueIndexes.Add(i);
      if (DialogueIndexes.Num() == ChildrenAmount) break;
    }
    if (DialogueDataArr[i].Action == BranchAction) IgnoreNext = DialogueDataArr[i].BranchChildrenAmount;
  }

  return DialogueIndexes;
}
inline auto GetChildChoiceIndexes(const TArray<FDialogueData>& DialogueDataArr, int32 StartIndex) -> TArray<int32> {
  return GetDialogueTreeChildIndexes(DialogueDataArr, StartIndex, EDialogueType::Choice,
                                     EDialogueAction::StartPlayerChoice);
}
inline auto GetChildInquireIndexes(const TArray<FDialogueData>& DialogueDataArr, int32 StartIndex) -> TArray<int32> {
  return GetDialogueTreeChildIndexes(DialogueDataArr, StartIndex, EDialogueType::Inquire,
                                     EDialogueAction::StartPlayerInquire);
}

void UDialoguePlayerSystem::StartDialogue(const TArray<FDialogueData> _DialogueDataArr, const FString& _SpeakerName) {
  check(_DialogueDataArr.Num() > 0);

  ResetDialogue();

  DialogueState = GetNextDialogueState(DialogueState, EDialogueAction::NextDialogue);
  SpeakerName = FText::FromString(_SpeakerName);
  DialogueDataArr = _DialogueDataArr;
  CurrDialogueIndex = 0;
}
void UDialoguePlayerSystem::StartDialogue(class UDialogueComponent* _DialogueC) {
  check(_DialogueC && _DialogueC->DialogueArray.Num() > 0);

  ResetDialogue();

  DialogueC = _DialogueC;
  DialogueState = GetNextDialogueState(DialogueState, EDialogueAction::NextDialogue);
  SpeakerName = DialogueC->SpeakerName;
  DialogueDataArr = DialogueC->GetNextDialogueChain();
  CurrDialogueIndex = 0;

  UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
  AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
  // ControlHUD->UpdateAndOpenDialogue();
}

void UDialoguePlayerSystem::NextDialogue() {
  DialogueState = GetNextDialogueState(DialogueState, DialogueDataArr[CurrDialogueIndex].Action);

  if (CurrDialogueIndex + 1 < DialogueDataArr.Num() &&
      DialogueDataArr[CurrDialogueIndex + 1].DialogueType == EDialogueType::Pointer) {
    FName PointerID = DialogueDataArr[CurrDialogueIndex + 1].PointerID;
    int32 PointerIndex = DialogueDataArr.IndexOfByPredicate(
        [PointerID](const FDialogueData& Dialogue) { return Dialogue.DialogueID == PointerID; });
    check(PointerIndex != INDEX_NONE);

    CurrDialogueIndex = PointerIndex;
  }

  switch (DialogueState) {
    case EDialogueState::Dialogue: {
      CurrDialogueIndex++;
      break;
    }
    case EDialogueState::PlayerChoice: {
      break;
    }
    case EDialogueState::PlayerInquire: {
      InquireBlockIndexes.Add(CurrDialogueIndex);

      break;
    }
    case EDialogueState::EndPlayerInquire: {
      check(InquireBlockIndexes.Num() > 0);
      InquireBlockIndexes.Pop();

      if (InquireBlockIndexes.Num() > 0) {
        CurrDialogueIndex = InquireBlockIndexes.Last();
        DialogueState = EDialogueState::PlayerInquire;
        break;
      }

      DialogueState = EDialogueState::End;
      if (DialogueC) DialogueC->FinishReadingDialogueChain();
      break;
    }
    case EDialogueState::End: {
      if (InquireBlockIndexes.Num() > 0) {
        CurrDialogueIndex = InquireBlockIndexes.Last();
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

auto UDialoguePlayerSystem::GetChoiceDialogues() -> TArray<FDialogueData> {
  if (DialogueState != EDialogueState::PlayerChoice) return {};

  const TArray<int32>& ChildChoiceIndexes = GetChildChoiceIndexes(DialogueDataArr, CurrDialogueIndex);
  TArray<FDialogueData> ChoiceDialogueArr = {};
  ChoiceDialogueArr.Reserve(ChildChoiceIndexes.Num());
  for (int32 ChoiceIndex : ChildChoiceIndexes) ChoiceDialogueArr.Add(DialogueDataArr[ChoiceIndex]);

  return ChoiceDialogueArr;
}
void UDialoguePlayerSystem::DialogueChoice(int32 ChoiceIndex) {
  check(ChoiceIndex >= 0 && ChoiceIndex < DialogueDataArr[CurrDialogueIndex].BranchChildrenAmount);
  check(DialogueState == EDialogueState::PlayerChoice);

  const TArray<int32>& ChildChoiceIndexes = GetChildChoiceIndexes(DialogueDataArr, CurrDialogueIndex);
  int32 ChoiceDialogueIndex = ChildChoiceIndexes[ChoiceIndex];

  CurrDialogueIndex = ChoiceDialogueIndex;
  NextDialogue();
}

auto UDialoguePlayerSystem::GetInquireDialogues() -> TArray<FDialogueData> {
  check(DialogueState == EDialogueState::PlayerInquire);

  const TArray<int32>& ChildInquireIndexes = GetChildInquireIndexes(DialogueDataArr, CurrDialogueIndex);
  TArray<FDialogueData> InquireDialogueArr = {};
  InquireDialogueArr.Reserve(ChildInquireIndexes.Num());
  for (int32 InquireIndex : ChildInquireIndexes) InquireDialogueArr.Add(DialogueDataArr[InquireIndex]);

  return InquireDialogueArr;
}
void UDialoguePlayerSystem::InquireDialogue(int32 InquireIndex) {
  check(InquireIndex >= 0 && InquireIndex < DialogueDataArr[CurrDialogueIndex].BranchChildrenAmount);
  check(DialogueState == EDialogueState::PlayerInquire);

  const TArray<int32>& ChildInquireIndexes = GetChildInquireIndexes(DialogueDataArr, CurrDialogueIndex);
  CurrDialogueIndex = ChildInquireIndexes[InquireIndex];
  NextDialogue();
}

void UDialoguePlayerSystem::ResetDialogue() {
  DialogueC = nullptr;
  DialogueState = EDialogueState::None;
  DialogueDataArr.Empty();
  CurrDialogueIndex = 0;
  InquireBlockIndexes.Empty();
}