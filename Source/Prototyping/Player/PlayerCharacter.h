
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Prototyping/Framework/GameStateStructs.h"
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

  UPROPERTY(EditAnywhere)
  class UGameStateSubsystem* GameStateSubsystem;
  void HandleGlobalGameStateChanged(GlobalGameState OldGameState, GlobalGameState NewGameState);

  // * Input
  UPROPERTY(EditAnywhere)
  TMap<GlobalGameState, class UInputMappingContext*> InputContexts;
  UPROPERTY(EditAnywhere)
  FInGameInputActions InGameInputActions;
  UPROPERTY(EditAnywhere)
  FInUIInputActions InUIInputActions;

  // * HUD
  UPROPERTY()
  class AInGameControlHUD* ControlHUD;

  // * Input functions
  // In Game
  UFUNCTION(BlueprintCallable)
  void OpenPauseMenu(const FInputActionValue& Value);
  // In Game
  UFUNCTION(BlueprintCallable)
  void Move(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void StartMove(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void StopMove(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void Interact(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void CinematicView(const FInputActionValue& Value);
  // In UI
  UFUNCTION(BlueprintCallable)
  void AdvanceUI(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void AdvanceUIHold(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void RetractUIAction(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void QuitUIAction(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UINumericInputAction(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UIDirectionalInputAction(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UISideButton1Action(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UISideButton2Action(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UISideButton3Action(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UISideButton4Action(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UICycleLeftAction(const FInputActionValue& Value);
  UFUNCTION(BlueprintCallable)
  void UICycleRightAction(const FInputActionValue& Value);

  // * Camera
  UPROPERTY(EditAnywhere)
  class USpringArmComponent* SpringArmC;
  UPROPERTY(EditAnywhere)
  class UCameraComponent* CameraC;

  // * Components
  // UPROPERTY(EditAnywhere, Category = "Character | Components")
  // class UInventoryComponent* PlayerInventoryComponent;
  // UPROPERTY(EditAnywhere, Category = "Character | Components")
  // class UTagsComponent* PlayerTagsComponent;  // Flags
  // UPROPERTY(EditAnywhere, Category = "Character | Components")
  // class UWidgetComponent* PlayerWidgetComponent;

  // * Movement
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector FacingDirection;
};
