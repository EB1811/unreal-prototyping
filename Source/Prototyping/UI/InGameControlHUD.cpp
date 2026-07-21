#include "InGameControlHUD.h"
#include "Logging/LogVerbosity.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/UI/InGameHud/InGameHudWidget.h"
#include "Prototyping/UI/PauseMenu/PauseMenuViewWidget.h"
#include "Prototyping/UI/TestHudWidget.h"
#include "Prototyping/Framework/Subsystems/ControlHUDSubsystem.h"
#include "Components/PanelWidget.h"
#include "TimerManager.h"

inline auto IsWidgetChildOf(UUserWidget* Widget, UUserWidget* Parent) -> bool {
  check(Widget);
  check(Parent);
  if (Widget == Parent) return true;
  if (!Parent->WidgetTree) return false;

  UWidget* FoundWidget = Parent->WidgetTree->FindWidget(Widget->GetFName());
  return (FoundWidget == Widget);
}

AInGameControlHUD::AInGameControlHUD() {
  HUDState = EHUDState::InGame;

  WidgetsRefreshInterval = 2.0f;
}

void AInGameControlHUD::DrawHUD() { Super::DrawHUD(); }

void AInGameControlHUD::BeginPlay() {
  Super::BeginPlay();

  const FInputModeGameOnly InputMode;
  GetOwningPlayerController()->SetInputMode(InputMode);
  GetOwningPlayerController()->SetShowMouseCursor(false);

  UIShowAnimCompleteEvent.BindDynamic(this, &AInGameControlHUD::UIShowAnimComplete);
  UIHideAnimCompleteEvent.BindDynamic(this, &AInGameControlHUD::UIHideAnimComplete);

  GetWorld()->GetTimerManager().SetTimer(TickRefreshingWidgetsTimerHandle, this,
                                         &AInGameControlHUD::TickRefreshingWidgets, WidgetsRefreshInterval, true,
                                         WidgetsRefreshInterval);
}

void AInGameControlHUD::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AInGameControlHUD::InitUIWidgets() {
  check(InGameHudWidgetClass);
  check(PauseMenuViewWidgetClass);

  InGameHudWidget = CreateWidget<UInGameHudWidget>(GetWorld(), InGameHudWidgetClass);
  InGameHudWidget->AddToViewport(1);
  InGameHudWidget->SetVisibility(ESlateVisibility::Hidden);

  PauseMenuViewWidget = CreateWidget<UPauseMenuViewWidget>(GetWorld(), PauseMenuViewWidgetClass);
  PauseMenuViewWidget->AddToViewport(50);
  PauseMenuViewWidget->SetVisibility(ESlateVisibility::Collapsed);

  TestHudWidget = CreateWidget<UTestHudWidget>(GetWorld(), TestHudWidgetClass);
  TestHudWidget->AddToViewport(100);
  TestHudWidget->SetVisibility(ESlateVisibility::Collapsed);

  InGameHudWidget->InitUI(InGameInputActions);
  PauseMenuViewWidget->InitUI(InUIInputActions);
}

inline FUIBehaviour* GetUIBehaviour(UUserWidget* Widget) {
  return GetOptReflectedProp<FUIBehaviour>(Widget, "UIBehaviour");
}
void AInGameControlHUD::ShowWidget(UUserWidget* Widget) {
  if (FUIBehaviour* UIBehaviour = GetUIBehaviour(Widget)) {
    HUDAnimState = EHUDAnimState::PlayingAnim;

    check(UIBehaviour->ShowAnim);
    UWidgetAnimation* ShowAnim = UIBehaviour->ShowAnim;
    Widget->UnbindAllFromAnimationFinished(ShowAnim);
    Widget->BindToAnimationFinished(ShowAnim, UIShowAnimCompleteEvent);

    Widget->SetVisibility(ESlateVisibility::Visible);
    Widget->PlayAnimation(ShowAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);

    USoundBase* OpenSound = UIBehaviour->OpenSound;
    if (OpenSound) UGameplayStatics::PlaySound2D(this, OpenSound, 1.0f);
    return;
  }

  Widget->SetVisibility(ESlateVisibility::Visible);
}
void AInGameControlHUD::HideWidget(UUserWidget* Widget) {
  if (FUIBehaviour* UIBehaviour = GetUIBehaviour(Widget)) {
    HUDAnimState = EHUDAnimState::PlayingAnim;
    UIHideAnimCompleteFunc = [this, Widget]() { Widget->SetVisibility(ESlateVisibility::Collapsed); };

    check(UIBehaviour->HideAnim);
    UWidgetAnimation* HideAnim = UIBehaviour->HideAnim;
    Widget->UnbindAllFromAnimationFinished(HideAnim);
    Widget->BindToAnimationFinished(HideAnim, UIHideAnimCompleteEvent);
    Widget->PlayAnimation(HideAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);

    USoundBase* HideSound = UIBehaviour->HideSound;
    if (HideSound) UGameplayStatics::PlaySound2D(this, HideSound, 1.0f);

    return;
  }

  Widget->SetVisibility(ESlateVisibility::Collapsed);
}
void AInGameControlHUD::UIShowAnimComplete() {
  if (UIShowAnimCompleteFunc) UIShowAnimCompleteFunc();

  HUDAnimState = EHUDAnimState::None;

  UE_LOG(LogTemp, Warning, TEXT("UIShowAnimComplete called"));
}
void AInGameControlHUD::UIHideAnimComplete() {
  if (UIHideAnimCompleteFunc) UIHideAnimCompleteFunc();

  HUDAnimState = EHUDAnimState::None;

  UE_LOG(LogTemp, Warning, TEXT("UIHideAnimComplete called"));
}

void AInGameControlHUD::OpenViewWidget(UUserWidget* Widget) {
  check(Widget);

  OpenedViewWidgets.Add(Widget);
  if (OpenedViewWidgets.Num() <= 1) {
    HUDState = EHUDState::FocusedMenu;

    const FInputModeGameAndUI InputMode;
    GetOwningPlayerController()->SetInputMode(InputMode);
    GetOwningPlayerController()->SetShowMouseCursor(true);
  }

  ShowWidget(Widget);
}
void AInGameControlHUD::CloseViewWidget(UUserWidget* Widget) {
  check(Widget);
  if (!OpenedViewWidgets.Contains(Widget)) return;

  TArray<UUserWidget*> RefreshingWidgetToRemove;
  for (UUserWidget* RefreshingWidget : RefreshingWidgets)
    if (IsWidgetChildOf(RefreshingWidget, Widget)) RefreshingWidgetToRemove.Add(RefreshingWidget);
  for (UUserWidget* RefreshingWidget : RefreshingWidgetToRemove) RefreshingWidgets.Remove(RefreshingWidget);

  OpenedViewWidgets.Remove(Widget);
  if (OpenedViewWidgets.IsEmpty()) {
    HUDState = EHUDState::InGame;

    const FInputModeGameOnly InputMode;
    GetOwningPlayerController()->SetInputMode(InputMode);
    GetOwningPlayerController()->SetShowMouseCursor(false);
  }

  HideWidget(Widget);
}

auto AInGameControlHUD::bUIAcceptingInput() const -> bool {
  // if (HUDState == EHUDState::PlayingAnim) return false;

  return true;
}
inline FUIActionable* GetUIActionable(UUserWidget* Widget) {
  return GetOptReflectedProp<FUIActionable>(Widget, "UIActionable");
}
void AInGameControlHUD::AdvanceUI(UUserWidget* ActionableWidget) {
  if (!bUIAcceptingInput()) return;

  check(ActionableWidget);
  FUIActionable* UIActionable = GetUIActionable(ActionableWidget);
  if (!UIActionable || !UIActionable->AdvanceUI) return;

  UIActionable->AdvanceUI();
}
void AInGameControlHUD::AdvanceUI() {
  if (OpenedViewWidgets.IsEmpty()) return;

  UUserWidget* TopWidget = OpenedViewWidgets.Last();
  AdvanceUI(TopWidget);
}
void AInGameControlHUD::UIDirectionalInputAction(FVector2D Direction, UUserWidget* ActionableWidget) {
  if (!bUIAcceptingInput()) return;

  check(ActionableWidget);
  FUIActionable* UIActionable = GetUIActionable(ActionableWidget);
  if (!UIActionable || !UIActionable->DirectionalInput) return;

  UIActionable->DirectionalInput(Direction);
}
void AInGameControlHUD::UIDirectionalInputAction(FVector2D Direction) {
  if (OpenedViewWidgets.IsEmpty()) return;

  UUserWidget* TopWidget = OpenedViewWidgets.Last();
  UIDirectionalInputAction(Direction, TopWidget);
}

inline FUIRefresh* GetUIRefresh(UUserWidget* Widget) { return GetOptReflectedProp<FUIRefresh>(Widget, "UIRefresh"); }
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
    UE_LOG(LogTemp, Log, TEXT("TickRefreshingWidgets: Refreshed widget %s"), *Widget->GetName());
  }
}

void AInGameControlHUD::ShowInGameHud() {}

void AInGameControlHUD::OpenPauseMenuView() {
  check(PauseMenuViewWidget);
  if (OpenedViewWidgets.Contains(PauseMenuViewWidget)) return;

  PauseMenuViewWidget->UpdateUI({
      .TestFloat = 42.0f,
  });
  PauseMenuViewWidget->RefreshUI();

  OpenViewWidget(PauseMenuViewWidget);
}

void AInGameControlHUD::OpenTestHudWidgetView() {
  check(TestHudWidget);
  if (OpenedViewWidgets.Contains(TestHudWidget)) return;

  OpenViewWidget(TestHudWidget);
}