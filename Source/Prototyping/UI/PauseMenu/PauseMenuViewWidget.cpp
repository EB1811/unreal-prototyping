#include "PauseMenuViewWidget.h"
#include "Prototyping/UI/Settings/SettingsWidget.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuViewWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuViewWidget::Resume);
  SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuViewWidget::OpenSettings);
  ResumeButton->OnHovered.AddDynamic(this, &UPauseMenuViewWidget::UnhoverButton);

  SetupUIActionable();
}

void UPauseMenuViewWidget::SelectHoveredButton() {
  if (!HoveredButton) return;

  if (HoveredButton == ResumeButton) Resume();
}
void UPauseMenuViewWidget::HoverButton(UButton* Button) {
  check(Button);
  if (HoveredButton == Button) return;

  if (HoveredButton) {
    FButtonStyle ButtonStyle = Button->GetStyle();
    ButtonStyle.SetNormal(ButtonStyle.Normal);
    HoveredButton->SetStyle(ButtonStyle);
  }
  HoveredButton = Button;
  FButtonStyle ButtonStyle = HoveredButton->GetStyle();
  ButtonStyle.SetNormal(ButtonStyle.Hovered);
  HoveredButton->SetStyle(ButtonStyle);
  // HoveredButton->SetFocus();

  UGameplayStatics::PlaySound2D(this, HoverSound, 1.0f);
}
void UPauseMenuViewWidget::HoverNextButton(FVector2D Direction) {
  if (Direction.X == 0) return;

  if (!HoveredButton) {
    HoverButton(ResumeButton);
    return;
  }

  TArray<UButton*> Buttons = {ResumeButton};
  int32 CurrentIndex = Buttons.IndexOfByKey(HoveredButton);
  check(CurrentIndex != INDEX_NONE);
  int32 NextIndex = CurrentIndex + (Direction.X > 0 ? 1 : -1);
  if (NextIndex < 0 || NextIndex >= Buttons.Num()) return;

  HoverButton(Buttons[NextIndex]);
}
void UPauseMenuViewWidget::UnhoverButton() {
  if (!HoveredButton) return;

  // FButtonStyle ButtonStyle = HoveredButton == ResumeButton ? SaveButton->GetStyle() : ResumeButton->GetStyle();
  // ButtonStyle.SetNormal(ButtonStyle.Normal);
  // HoveredButton->SetStyle(ButtonStyle);
  // HoveredButton = nullptr;
}

void UPauseMenuViewWidget::Resume() { UE_LOG(LogTemp, Log, TEXT("Resume button clicked")); }
void UPauseMenuViewWidget::OpenSettings() {
  UE_LOG(LogTemp, Log, TEXT("Settings button clicked"));

  SettingsWidget->UpdateUI();
  SettingsWidget->RefreshUI();

  UControlHUDSubsystem* ControlHUDSubsystem = GetWorld()->GetSubsystem<UControlHUDSubsystem>();
  check(ControlHUDSubsystem);
  AInGameControlHUD* ControlHUD = Cast<AInGameControlHUD>(ControlHUDSubsystem->GetHUD());
  check(ControlHUD);
  ControlHUD->ShowWidget(SettingsWidget);
}

void UPauseMenuViewWidget::RefreshUI() { MainOverlay->SetVisibility(ESlateVisibility::Visible); }

void UPauseMenuViewWidget::UpdateUI(FTestUIStruct _TestStruct) { TestStruct = _TestStruct; }

void UPauseMenuViewWidget::InitUI(FInUIInputActions _InUIInputActions) {
  InUIInputActions = _InUIInputActions;

  SettingsWidget->SetVisibility(ESlateVisibility::Collapsed);

  HoverButton(ResumeButton);
}

void UPauseMenuViewWidget::SetupUIActionable() {}
void UPauseMenuViewWidget::SetupUIBehaviour() {
  UIBehaviour.ShowAnim = ShowAnim;
  UIBehaviour.HideAnim = HideAnim;
  UIBehaviour.OpenSound = OpenSound;
  UIBehaviour.HideSound = HideSound;
}