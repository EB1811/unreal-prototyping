// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Prototyping/Player/InputStructs.h"
#include "Prototyping/UI/UIStructs.h"
#include "PauseMenuViewWidget.generated.h"

USTRUCT()
struct FTestUIStruct {
  GENERATED_BODY()

  float TestFloat;
};

UCLASS()
class PROTOTYPING_API UPauseMenuViewWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeOnInitialized() override;

  UPROPERTY(meta = (BindWidget))
  class UOverlay* MainOverlay;
  UPROPERTY(meta = (BindWidget))
  class USettingsWidget* SettingsWidget;

  UPROPERTY(meta = (BindWidget))
  class UButton* ResumeButton;
  UPROPERTY(meta = (BindWidget))
  class UButton* SettingsButton;

  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* ShowAnim;
  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* HideAnim;
  UPROPERTY(EditAnywhere)
  class USoundBase* OpenSound;
  UPROPERTY(EditAnywhere)
  class USoundBase* HideSound;

  UPROPERTY(EditAnywhere)
  class USoundBase* HoverSound;
  UPROPERTY(EditAnywhere)
  class USoundBase* SelectSound;

  UPROPERTY(EditAnywhere)
  FInUIInputActions InUIInputActions;

  UPROPERTY(EditAnywhere)
  FTestUIStruct TestStruct;

  UPROPERTY(EditAnywhere)
  class UButton* HoveredButton;
  void SelectHoveredButton();
  void HoverButton(UButton* Button);
  void HoverNextButton(FVector2D Direction);
  UFUNCTION()
  void UnhoverButton();

  UFUNCTION()
  void Resume();
  UFUNCTION()
  void OpenSettings();

  void RefreshUI();
  void UpdateUI(FTestUIStruct _TestStruct);
  void InitUI(FInUIInputActions _InUIInputActions);

  UPROPERTY(EditAnywhere)
  FUIActionable UIActionable;
  void SetupUIActionable();

  UPROPERTY(EditAnywhere)
  FUIBehaviour UIBehaviour;
  void SetupUIBehaviour();
};