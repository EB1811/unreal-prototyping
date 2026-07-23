
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Prototyping/Framework/UtilFuncs.h"
#include "Prototyping/Interaction/InteractionComponent.h"
#include "Prototyping/Dialogue/DialogueComponent.h"
#include "Prototyping/Dialogue/DialoguePlayerSystem.h"
#include "Prototyping/UI/InGameControlHUD.h"
#include "Prototyping/Framework/Subsystems/GameStateSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"

APlayerCharacter::APlayerCharacter() {
  SpringArmC = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
  SpringArmC->SetupAttachment(RootComponent);
  CameraC = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
  CameraC->SetupAttachment(SpringArmC);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
    // In Game
    EnhancedInputComponent->BindAction(InGameInputActions.MoveAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::Move);
    EnhancedInputComponent->BindAction(InGameInputActions.MoveAction, ETriggerEvent::Started, this,
                                       &APlayerCharacter::StartMove);
    EnhancedInputComponent->BindAction(InGameInputActions.MoveAction, ETriggerEvent::Completed, this,
                                       &APlayerCharacter::StopMove);
    EnhancedInputComponent->BindAction(InGameInputActions.OpenPauseMenuAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::OpenPauseMenu);
    EnhancedInputComponent->BindAction(InGameInputActions.InteractAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::Interact);
    EnhancedInputComponent->BindAction(InGameInputActions.CinematicViewAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::CinematicView);
    // In UI
    EnhancedInputComponent->BindAction(InUIInputActions.AdvanceUIAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::AdvanceUI);
    EnhancedInputComponent->BindAction(InUIInputActions.AdvanceUIHoldAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::AdvanceUIHold);
    EnhancedInputComponent->BindAction(InUIInputActions.RetractUIAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::RetractUIAction);
    EnhancedInputComponent->BindAction(InUIInputActions.QuitUIAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::QuitUIAction);
    EnhancedInputComponent->BindAction(InUIInputActions.UINumericInputAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UINumericInputAction);
    EnhancedInputComponent->BindAction(InUIInputActions.UIDirectionalInputAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UIDirectionalInputAction);
    EnhancedInputComponent->BindAction(InUIInputActions.UISideButton1Action, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UISideButton1Action);
    EnhancedInputComponent->BindAction(InUIInputActions.UISideButton2Action, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UISideButton2Action);
    EnhancedInputComponent->BindAction(InUIInputActions.UISideButton3Action, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UISideButton3Action);
    EnhancedInputComponent->BindAction(InUIInputActions.UISideButton4Action, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UISideButton4Action);
    EnhancedInputComponent->BindAction(InUIInputActions.UICycleLeftAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UICycleLeftAction);
    EnhancedInputComponent->BindAction(InUIInputActions.UICycleRightAction, ETriggerEvent::Triggered, this,
                                       &APlayerCharacter::UICycleRightAction);
  }
}

void APlayerCharacter::BeginPlay() {
  Super::BeginPlay();

  GameStateSubsystem = GetSubsystem<UGameStateSubsystem>(GetWorld());
  GameStateSubsystem->GameStateChangedDelegate.AddUObject(this, &APlayerCharacter::HandleGlobalGameStateChanged);

  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  Subsystem->AddMappingContext(InputContexts[GameStateSubsystem->CurrGameState], 0);

  ControlHUD = Cast<AInGameControlHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
  ControlHUD->InGameInputActions = InGameInputActions;
  ControlHUD->InUIInputActions = InUIInputActions;
}

void APlayerCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (GameStateSubsystem->CurrGameState == EGlobalGameState::InGame &&
      GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionData.InteractionCheckFrequency)
    CheckForInteraction();
}

void APlayerCharacter::HandleGlobalGameStateChanged(EGlobalGameState OldGameState, EGlobalGameState NewGameState) {
  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
      GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  Subsystem->RemoveMappingContext(InputContexts[OldGameState]);
  Subsystem->AddMappingContext(InputContexts[NewGameState], 0);
}

void APlayerCharacter::Move(const FInputActionValue& Value) {
  const FVector2D MovementVector = Value.Get<FVector2D>();
  const FRotator Rotation = GetControlRotation();
  const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
  const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

  AddMovementInput(ForwardDirection, MovementVector.Y);
  AddMovementInput(RightDirection, MovementVector.X);
}
void APlayerCharacter::StartMove(const FInputActionValue& Value) {
  // FootstepAudio->Activate();
  // GetWorld()->GetTimerManager().SetTimer(FootstepAudioTimer, this, &APlayerCharacter::PlayFootstepAudio, 0.32f, true);
}
void APlayerCharacter::StopMove(const FInputActionValue& Value) {
  // // FootstepAudio->Deactivate();
  // GetWorld()->GetTimerManager().ClearTimer(FootstepAudioTimer);
}

void APlayerCharacter::Interact(const FInputActionValue& Value) {}
void APlayerCharacter::CinematicView(const FInputActionValue& Value) {}

void APlayerCharacter::OpenPauseMenu(const FInputActionValue& Value) {
  if (!ControlHUD) return;

  ControlHUD->OpenPauseMenuView();
}
void APlayerCharacter::AdvanceUI(const FInputActionValue& Value) { ControlHUD->AdvanceUIAction(); }
void APlayerCharacter::AdvanceUIHold(const FInputActionValue& Value) { ControlHUD->AdvanceUIHoldAction(); }
void APlayerCharacter::RetractUIAction(const FInputActionValue& Value) { ControlHUD->RetractUIAction(); }
void APlayerCharacter::QuitUIAction(const FInputActionValue& Value) { ControlHUD->QuitUIAction(); }
void APlayerCharacter::UINumericInputAction(const FInputActionValue& Value) {
  ControlHUD->UINumericInputAction(Value.Get<float>());
}
void APlayerCharacter::UIDirectionalInputAction(const FInputActionValue& Value) {
  ControlHUD->UIDirectionalInputAction(Value.Get<FVector2D>());
}
void APlayerCharacter::UISideButton1Action(const FInputActionValue& Value) { ControlHUD->UISideButton1Action(); }
void APlayerCharacter::UISideButton2Action(const FInputActionValue& Value) { ControlHUD->UISideButton2Action(); }
void APlayerCharacter::UISideButton3Action(const FInputActionValue& Value) { ControlHUD->UISideButton3Action(); }
void APlayerCharacter::UISideButton4Action(const FInputActionValue& Value) { ControlHUD->UISideButton4Action(); }
void APlayerCharacter::UICycleLeftAction(const FInputActionValue& Value) { ControlHUD->UICycleLeftAction(); }
void APlayerCharacter::UICycleRightAction(const FInputActionValue& Value) { ControlHUD->UICycleRightAction(); }

auto APlayerCharacter::CheckForInteraction() -> bool {
  InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

  FVector TraceStart{GetPawnViewLocation() - FVector(0, 0, 50)};
  FVector TraceEnd{TraceStart + GetActorForwardVector() * InteractionData.InteractionCheckDistance};
  FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionData.InteractionCheckRadius);

  DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 0.1f, 0, 5.0f);
  FHitResult DebugHit;
  TArray<AActor*> DebugActorsToIgnore;
  DebugActorsToIgnore.Add(this);
  UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, Sphere.GetSphereRadius(),
                                          UEngineTypes::ConvertToTraceType(ECC_Visibility), false, DebugActorsToIgnore,
                                          EDrawDebugTrace::ForDuration, DebugHit, true, FLinearColor::Red,
                                          FLinearColor::Green, InteractionData.InteractionCheckFrequency);

  FCollisionQueryParams TraceParams;
  TraceParams.AddIgnoredActor(this);
  FHitResult TraceHit;

  if (GetWorld()->SweepSingleByChannel(TraceHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, Sphere,
                                       TraceParams)) {
    if ((TraceStart - TraceHit.ImpactPoint).Size() <= InteractionData.InteractionCheckDistance)
      if (UInteractionComponent* Interactable = TraceHit.GetActor()->FindComponentByClass<UInteractionComponent>()) {
        if (!IsInteractable(Interactable)) return false;

        CurrentInteractableC = Interactable;
        // if (!bInCinematicView) {
        //   PlayerWidgetComponent->SetWorldLocation(GetActorLocation() + FVector(0, -35.0f, 110.0f));
        //   PlayerWidgetComponent->SetVisibility(true, true);
        // }
        return true;
      }
  }

  if (CurrentInteractableC) CurrentInteractableC = nullptr;

  return false;
}
auto APlayerCharacter::CheckForPrioritisedInteraction() -> bool {
  InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

  FVector TraceStart{GetPawnViewLocation() - FVector(0, 0, 50)};
  FVector TraceEnd{TraceStart + GetActorForwardVector() * InteractionData.InteractionCheckDistance};
  FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionData.InteractionCheckRadius);

  FCollisionQueryParams TraceParams;
  TraceParams.AddIgnoredActor(this);
  TArray<FHitResult> TraceHits;

  bool bHit = GetWorld()->SweepMultiByChannel(TraceHits, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, Sphere,
                                              TraceParams);
  if (!bHit) {
    if (CurrentInteractableC) CurrentInteractableC = nullptr;
    // PlayerWidgetComponent->SetVisibility(false, true);
    return false;
  }

  auto FilteredTraceHits = TraceHits.FilterByPredicate([&](const FHitResult& Hit) {
    return ((TraceStart - Hit.ImpactPoint).Size() <= InteractionData.InteractionCheckDistance) && Hit.GetActor() &&
           Hit.GetActor()->FindComponentByClass<UInteractionComponent>();
  });
  if (FilteredTraceHits.Num() <= 0) {
    if (CurrentInteractableC) CurrentInteractableC = nullptr;
    // PlayerWidgetComponent->SetVisibility(false, true);
    return false;
  }
  UE_LOG(LogTemp, Warning, TEXT("Found %d interactable hits."), FilteredTraceHits.Num());

  // Sort based on priority and distance.
  FilteredTraceHits.Sort([&](const FHitResult& A, const FHitResult& B) {
    auto* AInteractable = A.GetActor()->FindComponentByClass<UInteractionComponent>();
    auto* BInteractable = B.GetActor()->FindComponentByClass<UInteractionComponent>();
    if (AInteractable && BInteractable) {
      int32 APrio = InteractionData.InteractionPriorities.Contains(AInteractable->InteractionType)
                        ? InteractionData.InteractionPriorities[AInteractable->InteractionType]
                        : 0;
      int32 BPrio = InteractionData.InteractionPriorities.Contains(BInteractable->InteractionType)
                        ? InteractionData.InteractionPriorities[BInteractable->InteractionType]
                        : 0;

      if ((APrio > 0 || BPrio > 0) && (APrio != BPrio)) return APrio > BPrio;
    }

    return (TraceStart - A.ImpactPoint).SizeSquared() < (TraceStart - B.ImpactPoint).SizeSquared();
  });

  auto TraceHit = FilteredTraceHits[0];
  UInteractionComponent* Interactable = TraceHit.GetActor()->FindComponentByClass<UInteractionComponent>();
  if (!IsInteractable(Interactable)) return false;

  CurrentInteractableC = Interactable;
  // if (!bInCinematicView) {
  //   PlayerWidgetComponent->SetWorldLocation(GetActorLocation() + FVector(0, -35.0f, 110.0f));
  //   PlayerWidgetComponent->SetVisibility(true, true);
  // }
  return true;
}
auto APlayerCharacter::IsInteractable(const UInteractionComponent* Interactable) const -> bool {
  check(Interactable);
  if (GameStateSubsystem->CurrGameState != EGlobalGameState::InGame) return false;

  switch (Interactable->InteractionType) {
    case EInteractionType::None: return false; break;
    default: break;
  }

  return true;
}
void APlayerCharacter::HandleInteraction(UInteractionComponent* Interactable) {
  // PlayerWidgetComponent->SetVisibility(false, true);

  Interactable->PlayInteractionSound();
  switch (Interactable->InteractionType) {
    case EInteractionType::Use: {
      Interactable->InteractUse();
      break;
    }
    case EInteractionType::Dialogue: {
      auto DialogueC = Interactable->InteractDialogue();
      // EnterDialogue(DialogueC.GetValue());
      break;
    }
    default: checkNoEntry();
  }
}