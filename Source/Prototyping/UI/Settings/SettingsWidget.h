// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Prototyping/Player/InputStructs.h"
#include "Prototyping/UI/UIStructs.h"
#include "SettingsWidget.generated.h"

UENUM()
enum class ESettingsCategory : uint8 {
  None UMETA(DisplayName = "None"),
  Game UMETA(DisplayName = "Game"),
  Display UMETA(DisplayName = "Display"),
  Graphics UMETA(DisplayName = "Graphics"),
  Sound UMETA(DisplayName = "Sound"),
  Controls UMETA(DisplayName = "Controls"),
};

UCLASS()
class PROTOTYPING_API USettingsWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeOnInitialized() override;

  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* ShowAnim;
  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* HideAnim;

  UPROPERTY(EditAnywhere)
  class USoundBase* HoverSound;
  UPROPERTY(EditAnywhere)
  class USoundBase* SelectSound;

  void RefreshUI();
  void UpdateUI();
  void InitUI(FInUIInputActions _InUIInputActions);

  UPROPERTY(EditAnywhere)
  FUIActionable UIActionable;
  void SetupUIActionable();

  UPROPERTY(EditAnywhere)
  FUIBehaviour UIBehaviour;
  void SetupUIBehaviour();

  UPROPERTY(EditAnywhere)
  FUIRefresh UIRefresh;
  void SetupUIRefresh();
};