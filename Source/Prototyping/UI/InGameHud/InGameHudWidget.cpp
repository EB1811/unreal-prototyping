#include "InGameHudWidget.h"
#include "Prototyping/UI/TestHudWidget.h"

void UInGameHudWidget::NativeOnInitialized() { Super::NativeOnInitialized(); }

void UInGameHudWidget::RefreshUI() {}

void UInGameHudWidget::InitUI(FInGameInputActions _InGameInputActions) { InGameInputActions = _InGameInputActions; }

void UInGameHudWidget::Show() {
  SetVisibility(ESlateVisibility::Visible);
  PlayAnimation(ShowAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
}
void UInGameHudWidget::Hide() { PlayAnimation(HideAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f); }
void UInGameHudWidget::HideAnimComplete() {}
