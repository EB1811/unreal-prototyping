#include "ControlHUDSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFramework/HUD.h"

void UControlHUDSubsystem::RegisterHUD(class AHUD* HUD) {
  check(HUD);

  CurrentControlHUD = HUD;
}

auto UControlHUDSubsystem::GetHUD() const -> class AHUD* {
  check(CurrentControlHUD);

  return CurrentControlHUD;
}