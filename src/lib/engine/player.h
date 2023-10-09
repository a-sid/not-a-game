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

struct Color {
  uint16_t Hue;
};

class Event;
class Lord;
class Player;

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

enum class PlayerConnectionState { NotReady, Ready, Online, Offline };

enum class PlayerSource { Human, AI };

using PlayerId = Id<Player>;
using LobbyPlayerId = Size;

struct Player {
  PlayerId MapId;
  LobbyPlayerId LobbyId;
  PlayerSource Source = PlayerSource::AI;
  std::string Name;
  Id<Fraction> FractionId;
  Id<Lord> LordId;
  Id<Icon> IconId;
  Id<MapObjectPtr> CapitalId;
  Color PlayerColor = Color{.Hue = 10000};
  Size TurnOrder = 10000;
  PlayerConnectionState State = PlayerConnectionState::NotReady;
};

} // namespace NotAGame
