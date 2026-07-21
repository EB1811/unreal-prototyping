#pragma once

#include "CoreMinimal.h"
#include "Animation/WidgetAnimationEvents.h"
#include "GameFramework/HUD.h"
#include "Prototyping/Player/InputStructs.h"
#include "InGameControlHUD.generated.h"

UENUM()
enum class EHUDState : uint8 {
  InGame UMETA(DisplayName = "InGame"),
  Paused UMETA(DisplayName = "Paused"),
  FocusedMenu UMETA(DisplayName = "FocusedMenu"),
  GameOver UMETA(DisplayName = "GameOver"),
};
UENUM()
enum class EHUDAnimState : uint8 {
  None UMETA(DisplayName = "None"),
  PlayingAnim UMETA(DisplayName = "PlayingAnim"),
};

UCLASS() class PROTOTYPING_API AInGameControlHUD : public AHUD {
  GENERATED_BODY()
public:
  AInGameControlHUD();
  virtual void DrawHUD() override;
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditAnywhere, Category = "Widgets", meta = (DisplayPriority = 1))
  TSubclassOf<class UInGameHudWidget> InGameHudWidgetClass;
  UPROPERTY(EditAnywhere, Category = "Widgets", meta = (DisplayPriority = 1))
  TSubclassOf<class UPauseMenuViewWidget> PauseMenuViewWidgetClass;
  UPROPERTY(EditAnywhere, Category = "Widgets", meta = (DisplayPriority = 1))
  TSubclassOf<class UTestHudWidget> TestHudWidgetClass;

  UPROPERTY(EditAnywhere)
  EHUDState HUDState;
  UPROPERTY(EditAnywhere)
  EHUDAnimState HUDAnimState;

  UPROPERTY(EditAnywhere)
  FInGameInputActions InGameInputActions;
  UPROPERTY(EditAnywhere)
  FInUIInputActions InUIInputActions;
  UPROPERTY(EditAnywhere)
  FInCutsceneInputActions InCutsceneInputActions;

  void InitUIWidgets();  // * Some stuff needs to be set up after all unreal funcs are called.

  // * Widget visibility and animation handling.
  void ShowWidget(class UUserWidget* Widget);
  void HideWidget(class UUserWidget* Widget);
  FWidgetAnimationDynamicEvent UIShowAnimCompleteEvent;
  UFUNCTION()
  void UIShowAnimComplete();
  TFunction<void()> UIShowAnimCompleteFunc;
  FWidgetAnimationDynamicEvent UIHideAnimCompleteEvent;
  UFUNCTION()
  void UIHideAnimComplete();
  TFunction<void()> UIHideAnimCompleteFunc;

  // * Widget management.
  UPROPERTY(EditAnywhere)
  TArray<class UUserWidget*> OpenedViewWidgets;

  void OpenViewWidget(class UUserWidget* Widget);
  void CloseViewWidget(class UUserWidget* Widget);

  // * Widget input handling.
  auto bUIAcceptingInput() const -> bool;
  void AdvanceUI(UUserWidget* ActionableWidget);
  void AdvanceUI();
  void UIDirectionalInputAction(FVector2D Direction, UUserWidget* ActionableWidget);
  void UIDirectionalInputAction(FVector2D Direction);

  // * Widget refreshing timer.
  UPROPERTY(EditAnywhere)
  TArray<class UUserWidget*> RefreshingWidgets;
  void AddToRefreshingWidgets(class UUserWidget* Widget);
  UPROPERTY(EditAnywhere)
  float WidgetsRefreshInterval;
  UPROPERTY(EditAnywhere)
  FTimerHandle TickRefreshingWidgetsTimerHandle;
  void TickRefreshingWidgets();

  // * Menu opening and closing.
  UPROPERTY()
  class UInGameHudWidget* InGameHudWidget;
  UFUNCTION(BlueprintCallable)
  void ShowInGameHud();

  UPROPERTY()
  class UPauseMenuViewWidget* PauseMenuViewWidget;
  UFUNCTION(BlueprintCallable)
  void OpenPauseMenuView();

  UPROPERTY()
  class UTestHudWidget* TestHudWidget;
  UFUNCTION(BlueprintCallable)
  void OpenTestHudWidgetView();
};