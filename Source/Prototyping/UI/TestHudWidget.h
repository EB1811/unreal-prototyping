// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Prototyping/Player/InputStructs.h"
#include "TestHudWidget.generated.h"

UCLASS()
class PROTOTYPING_API UTestHudWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UPROPERTY(meta = (BindWidget))
  class UTextBlock* TestTextBlock;
};