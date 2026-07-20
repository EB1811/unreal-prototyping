#include "SettingsWidget.h"
#include "CoreFwd.h"
#include "Logging/LogVerbosity.h"
#include "Misc/AssertionMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/UI/InGameControlHUD.h"

void USettingsWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  SetupUIActionable();
  SetupUIBehaviour();
  SetupUIRefresh();
}

void USettingsWidget::RefreshUI() {}

void USettingsWidget::UpdateUI() {
  UControlHUDSubsystem* ControlHUDSubsystem = GetSubsystem<UControlHUDSubsystem>(GetWorld());
  AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
  check(ControlHUD);
  ControlHUD->AddToRefreshingWidgets(this);
}

void USettingsWidget::InitUI(FInUIInputActions _InUIInputActions) {}

void USettingsWidget::SetupUIActionable() {
  UIActionable.AdvanceUI = [this]() {
    UE_LOG(LogTemp, Warning, TEXT("USettingsWidget::UIActionable.AdvanceUI called"));
  };
}
void USettingsWidget::SetupUIBehaviour() {
  UIBehaviour.ShowAnim = ShowAnim;
  UIBehaviour.HideAnim = HideAnim;
}
void USettingsWidget::SetupUIRefresh() {
  UIRefresh.RefreshTick = [this]() { UE_LOG(LogTemp, Log, TEXT("USettingsWidget::UIRefresh.RefreshTick called")); };
}