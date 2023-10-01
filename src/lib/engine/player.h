#pragma once

#include "entities/fraction.h"
#include "entities/global_map.h"
#include "entities/squad.h"
#include "entities/unit.h"
#include "util/types.h"

#include <cstdlib>
#include <string>

struct Color {
  uint8_t Hue;
};

namespace NotAGame {

constexpr Size kMaxPlayers = 16;

struct Color {
  uint16_t Hue;
};

class Event;
class Lord;

class PlayerActions {
  virtual void HandleEvent(const Event &E) = 0;
  virtual void EmitEvent(const Event &E) = 0;
};

inline Color ColorByTurnOrder(Size TurnOrder) noexcept {
  // Heroes-like table.
  if (TurnOrder < 8) {
    std::array<uint16_t, 8> Hues = {
        300, // Magenta, for neutral player.
        0,   // Red
        240, // Blue
        60,  // Yellow
        90,  // Green
        30,  // Orange
        270, // Violet
        180, // Cyan
    };
    return Color{Hues[TurnOrder]};
  }
  return Color{static_cast<uint16_t>(rand() % 360)};
}

enum class PlayerConnectionState { NotReady, Ready, Online, Offline };

struct Player {
  Size PlayerId;
  std::string Name;
  Id<Fraction> FractionId;
  Id<Lord> LordId;
  Id<Icon> IconId;
  Id<MapObjectPtr> CapitalId;
  Color PlayerColor;
  Size TurnOrder;
  PlayerConnectionState State = PlayerConnectionState::NotReady;
};

} // namespace NotAGame
