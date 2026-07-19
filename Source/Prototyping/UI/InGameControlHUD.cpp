#include "InGameControlHUD.h"
#include "Logging/LogVerbosity.h"
#include "Prototyping/UI/InGameHud/InGameHudWidget.h"
#include "Prototyping/UI/PauseMenu/PauseMenuViewWidget.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Components/PanelWidget.h"
#include "TimerManager.h"

inline auto IsWidgetChildOf(UWidget* Widget, UWidget* Parent) -> bool {
  check(Widget);
  check(Parent);
  if (Widget == Parent) return true;

  UWidget* CurrentParent = Widget->GetParent();
  while (CurrentParent) {
    if (CurrentParent == Parent) return true;
    CurrentParent = CurrentParent->GetParent();
  }

  return false;
}

AInGameControlHUD::AInGameControlHUD() {
  HUDState = EHUDState::InGame;

  WidgetsRefreshInterval = 2.0f;
}

void AInGameControlHUD::DrawHUD() { Super::DrawHUD(); }

void AInGameControlHUD::BeginPlay() {
  Super::BeginPlay();

  // Register this HUD with the ControlHUDSubsystem
  UControlHUDSubsystem* ControlHUDSubsystem = GetWorld()->GetSubsystem<UControlHUDSubsystem>();
  check(ControlHUDSubsystem);
  ControlHUDSubsystem->RegisterHUD(this);

  check(InGameHudWidgetClass);
  check(PauseMenuViewWidgetClass);

  InGameHudWidget = CreateWidget<UInGameHudWidget>(GetWorld(), InGameHudWidgetClass);
  InGameHudWidget->AddToViewport(1);
  InGameHudWidget->SetVisibility(ESlateVisibility::Hidden);

  PauseMenuViewWidget = CreateWidget<UPauseMenuViewWidget>(GetWorld(), PauseMenuViewWidgetClass);
  PauseMenuViewWidget->AddToViewport(50);
  PauseMenuViewWidget->SetVisibility(ESlateVisibility::Collapsed);

  InGameHudWidget->InitUI(InGameInputActions);
  PauseMenuViewWidget->InitUI(InUIInputActions);

  GetWorld()->GetTimerManager().SetTimer(TickRefreshingWidgetsTimerHandle, this,
                                         &AInGameControlHUD::TickRefreshingWidgets, WidgetsRefreshInterval, true,
                                         WidgetsRefreshInterval);
}

void AInGameControlHUD::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AInGameControlHUD::InitUIWidgets() {
  // InGameHudWidget->InitUI(InGameInputActions);
  // PauseMenuViewWidget->InitUI(InUIInputActions);
}

inline FUIBehaviour* GetUIBehaviour(UUserWidget* Widget) {
  FProperty* FUIBehaviourProp = Widget->GetClass()->FindPropertyByName("UIBehaviour");
  if (!FUIBehaviourProp) return nullptr;

  return FUIBehaviourProp->ContainerPtrToValuePtr<FUIBehaviour>(Widget);
}
void AInGameControlHUD::ShowWidget(UUserWidget* Widget) {
  if (!Widget) return;

  Widget->SetVisibility(ESlateVisibility::Visible);
}
void AInGameControlHUD::HideWidget(UUserWidget* Widget) {
  if (!Widget) return;

  Widget->SetVisibility(ESlateVisibility::Hidden);
}

auto AInGameControlHUD::bUIAcceptingInput() const -> bool {
  if (OpenedViewWidgets.IsEmpty()) return false;
  if (HUDState == EHUDState::PlayingAnim) return false;

  return true;
}

inline FUIActionable* GetUIActionable(UUserWidget* Widget) {
  FProperty* UIActionableProp = Widget->GetClass()->FindPropertyByName("UIActionable");
  if (!UIActionableProp) return nullptr;

  return UIActionableProp->ContainerPtrToValuePtr<FUIActionable>(Widget);
}
void AInGameControlHUD::AdvanceUI() {
  if (!bUIAcceptingInput()) return;

  UUserWidget* TopWidget = OpenedViewWidgets.Last();
  FUIActionable* ActionableWidget = GetUIActionable(TopWidget);
  if (!ActionableWidget || !ActionableWidget->AdvanceUI) return;

  ActionableWidget->AdvanceUI();
}

inline FUIRefresh* GetUIRefresh(UUserWidget* Widget) {
  FProperty* UIRefreshProp = Widget->GetClass()->FindPropertyByName("UIRefresh");
  if (!UIRefreshProp) return nullptr;

  return UIRefreshProp->ContainerPtrToValuePtr<FUIRefresh>(Widget);
}
void AInGameControlHUD::AddToRefreshingWidgets(UUserWidget* Widget) {
  if (RefreshingWidgets.Contains(Widget)) return;

  FUIRefresh* UIRefresh = GetUIRefresh(Widget);
  check(UIRefresh && UIRefresh->RefreshTick);
  RefreshingWidgets.Add(Widget);
}
void AInGameControlHUD::TickRefreshingWidgets() {
  for (UUserWidget* Widget : RefreshingWidgets) {
    FUIRefresh* UIRefresh = GetUIRefresh(Widget);
    check(UIRefresh && UIRefresh->RefreshTick);

    UIRefresh->RefreshTick();
  }
}

void AInGameControlHUD::ShowInGameHud() {
  if (!InGameHudWidget) return;

  ShowWidget(InGameHudWidget);
}

void AInGameControlHUD::OpenPauseMenuView() {
  if (!PauseMenuViewWidget) return;

  const FInputModeGameAndUI InputMode;
  GetOwningPlayerController()->SetInputMode(InputMode);
  GetOwningPlayerController()->SetShowMouseCursor(true);

  ShowWidget(PauseMenuViewWidget);
}