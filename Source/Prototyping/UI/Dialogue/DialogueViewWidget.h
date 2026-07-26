// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Prototyping/Player/InputStructs.h"
#include "Prototyping/UI/UIStructs.h"
#include "DialogueViewWidget.generated.h"

UCLASS()
class PROTOTYPING_API UDialogueViewWidget : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeOnInitialized() override;

  UPROPERTY(meta = (BindWidget))
  class UTextBlock* DialogueText;
  UPROPERTY(meta = (BindWidget))
  class UButton* NextButton;

  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* ShowAnim;
  UPROPERTY(Transient, meta = (BindWidgetAnim))
  class UWidgetAnimation* HideAnim;
  UPROPERTY(EditAnywhere)
  class USoundBase* OpenSound;
  UPROPERTY(EditAnywhere)
  class USoundBase* HideSound;

  UPROPERTY(EditAnywhere)
  class UDialoguePlayerSystem* DialoguePlayerSystem;

  UFUNCTION()
  void Next();
  UFUNCTION()
  void Select(int32 SelectIndex);
  UFUNCTION()
  void Close();

  void RefreshUI();
  void UpdateUI();
  void InitUI(class UDialoguePlayerSystem* _DialoguePlayerSystem);

  UPROPERTY(EditAnywhere)
  FUIActionable UIActionable;
  void SetupUIActionable();

  UPROPERTY(EditAnywhere)
  FUIBehaviour UIBehaviour;
  void SetupUIBehaviour();
};