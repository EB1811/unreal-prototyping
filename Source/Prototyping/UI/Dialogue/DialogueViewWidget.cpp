#include "DialogueViewWidget.h"
#include "Components/TextBlock.h"
#include "Prototyping/Dialogue/DialogueDataStructs.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/Dialogue/DialogueSystem.h"
#include "Prototyping/UI/Settings/SettingsWidget.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UDialogueViewWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  NextButton->OnClicked.AddDynamic(this, &UDialogueViewWidget::Next);

  SetupUIActionable();
  SetupUIBehaviour();
}

void UDialogueViewWidget::Next() {
  if (DialogueSystem->DialogueState != EDialogueState::Dialogue) return;

  DialogueSystem->NextDialogue();
  RefreshUI();
}
void UDialogueViewWidget::Select(int32 SelectIndex) {
  FDialogueData CurrDialogue = DialogueSystem->DialogueDataArr[DialogueSystem->CurrDialogueIndex];
  if (SelectIndex < 0 || SelectIndex >= CurrDialogue.BranchChildrenAmount) return;

  if (DialogueSystem->DialogueState == EDialogueState::PlayerChoice) DialogueSystem->DialogueChoice(SelectIndex);
  else if (DialogueSystem->DialogueState == EDialogueState::PlayerInquire) DialogueSystem->InquireDialogue(SelectIndex);
  else return;

  RefreshUI();
}
void UDialogueViewWidget::Close() {
  UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
  AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
  ControlHUD->CloseViewWidget(this);

  DialogueSystem->CloseDialogue();
}

void UDialogueViewWidget::RefreshUI() {
  switch (DialogueSystem->DialogueState) {
    case EDialogueState::PlayerChoice: {
      auto ChoiceDialogues = DialogueSystem->GetChoiceDialogues();
      DialogueText->SetText(
          FText::FromString(FString::JoinBy(ChoiceDialogues, TEXT("\n"), [](const FDialogueData& Dialogue) {
            return Dialogue.DialogueText.ToString();
          })));
      break;
    }
    case EDialogueState::PlayerInquire: {
      auto InquireDialogues = DialogueSystem->GetInquireDialogues();
      DialogueText->SetText(
          FText::FromString(FString::JoinBy(InquireDialogues, TEXT("\n"), [](const FDialogueData& Dialogue) {
            return Dialogue.DialogueText.ToString();
          })));
      break;
    }
    case EDialogueState::Dialogue: {
      FDialogueData CurrDialogue = DialogueSystem->DialogueDataArr[DialogueSystem->CurrDialogueIndex];
      DialogueText->SetText(CurrDialogue.DialogueText);
      break;
    }
    case EDialogueState::End: {
      UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
      AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
      ControlHUD->CloseViewWidget(this);

      DialogueSystem->FinishDialogue();
      break;
    }
    default: checkNoEntry(); break;
  }
}

void UDialogueViewWidget::UpdateUI() {}

void UDialogueViewWidget::InitUI(class ADialogueSystem* _DialogueSystem) {
  check(_DialogueSystem);
  DialogueSystem = _DialogueSystem;
}

void UDialogueViewWidget::SetupUIActionable() {
  UIActionable.AdvanceUI = [this]() { Next(); };
  UIActionable.NumericInput = [this](float Value) { Select(FMath::RoundToInt(Value) - 1); };
  UIActionable.RetractUI = [this]() { Close(); };
  UIActionable.QuitUI = [this]() { Close(); };
}
void UDialogueViewWidget::SetupUIBehaviour() {
  UIBehaviour.ShowAnim = ShowAnim;
  UIBehaviour.HideAnim = HideAnim;
  UIBehaviour.OpenSound = OpenSound;
  UIBehaviour.HideSound = HideSound;
}