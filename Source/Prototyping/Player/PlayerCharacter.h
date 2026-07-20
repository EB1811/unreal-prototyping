
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Prototyping/Player/InputStructs.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROTOTYPING_API APlayerCharacter : public ACharacter {
  GENERATED_BODY()

public:
  APlayerCharacter();

  // * Overrides
  virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  UPROPERTY()
  class AInGameControlHUD* ControlHUD;

  // * Input
  UPROPERTY(EditAnywhere)
  class UInputMappingContext* InputContext;
  UPROPERTY(EditAnywhere)
  FInGameInputActions InGameInputActions;
  UPROPERTY(EditAnywhere)
  class UInputMappingContext* InUIInputContext;
  UPROPERTY(EditAnywhere)
  FInUIInputActions InUIInputActions;

  // * Input functions
  // In Game
  UFUNCTION(BlueprintCallable)
  void OpenPauseMenu(const FInputActionValue& Value);
  // In UI
  UFUNCTION(BlueprintCallable)
  void AdvanceUI(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UIDirectionalInputAction(const FInputActionValue& Value);
};
