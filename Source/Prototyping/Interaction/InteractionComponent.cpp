// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionComponent.h"
#include "Prototyping/Dialogue/DialogueComponent.h"
#include "Kismet/GameplayStatics.h"

UInteractionComponent::UInteractionComponent() {
  PrimaryComponentTick.bCanEverTick = false;

  InteractionType = EInteractionType::Use;
}

void UInteractionComponent::BeginPlay() { Super::BeginPlay(); }

void UInteractionComponent::PlayInteractionSound() const {
  if (InteractionSound) UGameplayStatics::PlaySoundAtLocation(this, InteractionSound, GetOwner()->GetActorLocation());
}

void UInteractionComponent::StartInteraction() {
  if (bIsInteracting) return;

  bIsInteracting = true;
}
void UInteractionComponent::EndInteraction() {
  if (!bIsInteracting) return;

  bIsInteracting = false;
}

void UInteractionComponent::InteractUse() const {}
void UInteractionComponent::InteractExamine() const {}
auto UInteractionComponent::InteractDialogue() const -> TOptional<class UDialogueComponent*> {
  UDialogueComponent* OwnerDialogueC = GetOwner()->FindComponentByClass<UDialogueComponent>();
  check(OwnerDialogueC);

  return OwnerDialogueC;
}

// auto UInteractionComponent::InteractLevelChange() const -> ULevelChangeComponent* {
//   ULevelChangeComponent* OwnerLevelChangeC = GetOwner()->FindComponentByClass<ULevelChangeComponent>();
//   check(OwnerLevelChangeC);

//   return OwnerLevelChangeC;
// }

// auto UInteractionComponent::InteractContainer() const -> class UInventoryComponent* {
//   UInventoryComponent* OwnerInventoryC = GetOwner()->FindComponentByClass<UInventoryComponent>();
//   check(OwnerInventoryC);

//   return OwnerInventoryC;
// }

// auto UInteractionComponent::InteractPickup() const -> TOptional<class UPickupComponent*> {
//   UPickupComponent* OwnerPickupC = GetOwner()->FindComponentByClass<UPickupComponent>();
//   check(OwnerPickupC);

//   return OwnerPickupC;
// }