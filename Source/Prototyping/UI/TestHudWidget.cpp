#include "TestHudWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UTestHudWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  TestButton->OnClicked.AddDynamic(this, &UTestHudWidget::PressTestButton);
}

void UTestHudWidget::PressTestButton() {
  if (TestTextBlock) TestTextBlock->SetText(FText::FromString("Test Button Pressed!"));
}