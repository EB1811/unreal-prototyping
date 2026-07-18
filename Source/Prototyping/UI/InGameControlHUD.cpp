#include "InGameControlHUD.h"
#include "Logging/LogVerbosity.h"
#include "Prototyping/UI/InGameHud/InGameHudWidget.h"
#include "Prototyping/UI/PauseMenu/PauseMenuViewWidget.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "TimerManager.h"

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

void AInGameControlHUD::ShowWidget(UUserWidget* Widget) {
  if (!Widget) return;

  Widget->SetVisibility(ESlateVisibility::Visible);
}
void AInGameControlHUD::HideWidget(UUserWidget* Widget) {
  if (!Widget) return;

  Widget->SetVisibility(ESlateVisibility::Hidden);
}

void AInGameControlHUD::AddToRefreshingWidgets(UUserWidget* Widget) {
  FProperty* FUIBehaviourProp = Widget->GetClass()->FindPropertyByName("UIRefresh");
  check(FUIBehaviourProp);
  FUIRefresh* UIRefreshPtr = FUIBehaviourProp->ContainerPtrToValuePtr<FUIRefresh>(Widget);
  check(UIRefreshPtr->RefreshTick);

  if (RefreshingWidgets.Contains(Widget)) return;

  RefreshingWidgets.Add(Widget);
}
void AInGameControlHUD::TickRefreshingWidgets() {
  for (UUserWidget* Widget : RefreshingWidgets) {
    FProperty* FUIBehaviourProp = Widget->GetClass()->FindPropertyByName("UIRefresh");
    check(FUIBehaviourProp);
    FUIRefresh* UIRefreshPtr = FUIBehaviourProp->ContainerPtrToValuePtr<FUIRefresh>(Widget);
    check(UIRefreshPtr->RefreshTick);

    UIRefreshPtr->RefreshTick();
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