#include "DialogueComponent.h"
#include "Containers/Array.h"
#include "Engine/DataTable.h"
#include "Prototyping/Dialogue/DialogueDataStructs.h"

void UDialogueComponent::BeginPlay() { Super::BeginPlay(); }

auto UDialogueComponent::GetNextDialogueChain() -> TArray<FDialogueData> {
  switch (DialogueComponentType) {
    case EDialogueComponentType::Simple: {
      return DialogueArray;
    }
    case EDialogueComponentType::Rolling: {
      if (DialogueArray.Num() == 0) return {};

      if (CurrentDialogueChainId.IsNone()) CurrentDialogueChainId = DialogueArray[0].DialogueChainID;

      TArray<FDialogueData> NextDialogueChain;
      for (int32 i = 0; i < DialogueArray.Num(); i++)
        if (DialogueArray[i].DialogueChainID == CurrentDialogueChainId) NextDialogueChain.Add(DialogueArray[i]);

      check(NextDialogueChain.Num() > 0);
      return NextDialogueChain;
    }
    case EDialogueComponentType::Random: {
      if (DialogueArray.Num() == 0) return {};

      TArray<struct FDialogueData> Unique = DialogueArray.FilterByPredicate(
          [this](const FDialogueData& Dialogue) { return Dialogue.DialogueChainID != CurrentDialogueChainId; });
      CurrentDialogueChainId =
          !Unique.IsEmpty() ? Unique[FMath::RandRange(0, Unique.Num() - 1)].DialogueChainID : CurrentDialogueChainId;

      TArray<FDialogueData> NextDialogueChain;
      for (int32 i = 0; i < DialogueArray.Num(); i++)
        if (DialogueArray[i].DialogueChainID == CurrentDialogueChainId) NextDialogueChain.Add(DialogueArray[i]);

      check(NextDialogueChain.Num() > 0);
      return NextDialogueChain;
    }
    default: checkNoEntry(); return {};
  }
}

void UDialogueComponent::FinishReadingDialogueChain() {
  if (DialogueComponentType == EDialogueComponentType::Rolling) {
    int32 CurrentDialogueChainEndIndex = DialogueArray.IndexOfByPredicate(
        [this](const FDialogueData& Dialogue) { return Dialogue.DialogueChainID == CurrentDialogueChainId; });
    for (int32 i = CurrentDialogueChainEndIndex; i < DialogueArray.Num(); i++)
      if (DialogueArray[i].DialogueChainID == CurrentDialogueChainId) CurrentDialogueChainEndIndex = i;
      else break;

    if (CurrentDialogueChainEndIndex >= DialogueArray.Num() - 1) return;  // Final chain reached.

    CurrentDialogueChainId = DialogueArray[CurrentDialogueChainEndIndex + 1].DialogueChainID;
  }
}