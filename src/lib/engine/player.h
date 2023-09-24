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

constexpr Size kMaxPlayers = 8;

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
        0,   // Red
        240, // Blue
        60,  // Yellow
        90,  // Green
        30,  // Orange
        270, // Violet
        180, // Cyan
        300, // Magenta
    };
    return Color{Hues[TurnOrder]};
  }
  return Color{static_cast<uint16_t>(rand() % 360)};
}

struct Player {
  std::string Name;
  Id<Fraction> FractionId;
  Id<Lord> LordId;
  Id<Icon> IconId;
  Color PlayerColor;
};

} // namespace NotAGame
