// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Prototyping/Player/InputStructs.h"
#include "InGameHudWidget.generated.h"

UCLASS()
class PROTOTYPING_API UInGameHudWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeOnInitialized() override;

  UPROPERTY(meta = (BindWidget))
  class UTestHudWidget* TestHudWidget;

  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* ShowAnim;
  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* HideAnim;

  UPROPERTY(EditAnywhere)
  FInGameInputActions InGameInputActions;

  void RefreshUI();
  void InitUI(FInGameInputActions _InGameInputActions);

  void Show();
  void Hide();
  UFUNCTION()
  void HideAnimComplete();
};