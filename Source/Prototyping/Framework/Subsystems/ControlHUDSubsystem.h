
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ControlHUDSubsystem.generated.h"

// * Subsystem for widgets to perform actions on the HUD, e.g., playing animations, changing visibility, etc.
// Avoids direct references to the HUD from widgets.

UCLASS()
class PROTOTYPING_API UControlHUDSubsystem : public UWorldSubsystem {
  GENERATED_BODY()

public:
  UPROPERTY()
  class AHUD* CurrentControlHUD;

  void RegisterHUD(class AHUD* HUD);

  auto GetHUD() const -> class AHUD*;
};