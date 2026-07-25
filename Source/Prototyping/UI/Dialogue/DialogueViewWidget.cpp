#include "DialogueViewWidget.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/UI/Settings/SettingsWidget.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UDialogueViewWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  SetupUIActionable();
  SetupUIBehaviour();
}

void UDialogueViewWidget::RefreshUI() {}

void UDialogueViewWidget::UpdateUI() {}

void UDialogueViewWidget::InitUI() {}

void UDialogueViewWidget::SetupUIActionable() {}
void UDialogueViewWidget::SetupUIBehaviour() {
  UIBehaviour.ShowAnim = ShowAnim;
  UIBehaviour.HideAnim = HideAnim;
  UIBehaviour.OpenSound = OpenSound;
  UIBehaviour.HideSound = HideSound;
}