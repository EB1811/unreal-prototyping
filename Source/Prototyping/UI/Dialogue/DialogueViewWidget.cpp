#include "DialogueViewWidget.h"
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

void UDialogueViewWidget::RefreshUI() {
  switch (DialoguePlayerSystem->DialogueState) {
    case EDialogueState::PlayerChoice: {
      break;
    }
    case EDialogueState::PlayerInquire: {
      break;
    }
    case EDialogueState::Dialogue: {
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

void UDialogueViewWidget::SetupUIActionable() {}
void UDialogueViewWidget::SetupUIBehaviour() {
  UIBehaviour.ShowAnim = ShowAnim;
  UIBehaviour.HideAnim = HideAnim;
  UIBehaviour.OpenSound = OpenSound;
  UIBehaviour.HideSound = HideSound;
}