#include "DialogueViewWidget.h"
#include "Components/TextBlock.h"
#include "Prototyping/Dialogue/DialogueDataStructs.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/Dialogue/DialoguePlayerSystem.h"
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
  if (DialoguePlayerSystem->DialogueState != EDialogueState::Dialogue) return;

  DialoguePlayerSystem->NextDialogue();
  RefreshUI();
}
void UDialogueViewWidget::Select(int32 SelectIndex) {
  if (DialoguePlayerSystem->DialogueState == EDialogueState::PlayerChoice)
    DialoguePlayerSystem->DialogueChoice(SelectIndex);
  else if (DialoguePlayerSystem->DialogueState == EDialogueState::PlayerInquire)
    DialoguePlayerSystem->InquireDialogue(SelectIndex);
  else return;

  RefreshUI();
}
void UDialogueViewWidget::Close() {
  UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
  AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
  ControlHUD->CloseViewWidget(this);

  DialoguePlayerSystem->CloseDialogue();
}

void UDialogueViewWidget::RefreshUI() {
  switch (DialoguePlayerSystem->DialogueState) {
    case EDialogueState::PlayerChoice: {
      auto ChoiceDialogues = DialoguePlayerSystem->GetChoiceDialogues();
      DialogueText->SetText(
          FText::FromString(FString::JoinBy(ChoiceDialogues, TEXT("\n"), [](const FDialogueData& Dialogue) {
            return Dialogue.DialogueText.ToString();
          })));
      break;
    }
    case EDialogueState::PlayerInquire: {
      auto InquireDialogues = DialoguePlayerSystem->GetInquireDialogues();
      DialogueText->SetText(
          FText::FromString(FString::JoinBy(InquireDialogues, TEXT("\n"), [](const FDialogueData& Dialogue) {
            return Dialogue.DialogueText.ToString();
          })));
      break;
    }
    case EDialogueState::Dialogue: {
      FDialogueData CurrDialogue = DialoguePlayerSystem->DialogueDataArr[DialoguePlayerSystem->CurrDialogueIndex];
      DialogueText->SetText(CurrDialogue.DialogueText);
      break;
    }
    case EDialogueState::End: {
      UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
      AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
      ControlHUD->CloseViewWidget(this);

      DialoguePlayerSystem->FinishDialogue();
      break;
    }
    default: checkNoEntry(); break;
  }
}

void UDialogueViewWidget::UpdateUI() {}

void UDialogueViewWidget::InitUI(class UDialoguePlayerSystem* _DialoguePlayerSystem) {
  check(_DialoguePlayerSystem);
  DialoguePlayerSystem = _DialoguePlayerSystem;
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