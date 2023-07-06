#pragma once

#include "entities/unit.h"
#include "util/id.h"
#include "util/types.h"

#include <cstdint>
#include <string>

namespace NotAGame {

struct UnitPosition {
  Unit *Unit;
  Coord Position;
};

class Grid {
public:
  static constexpr Size WIDTH = 2;
  static constexpr Size HEIGHT = 3;

  Grid() noexcept {
    for (Dim Y = 0; Y < HEIGHT; ++Y) {
      for (Dim X = 0; X < WIDTH; ++X) {
        Units_[Y][X] = nullptr;
      }
    }
  }

  Unit *GetUnit(Coord Coord) const noexcept { return Units_[Coord.Y][Coord.X]; }

  bool CanPlaceUnit(Unit *Unit, Coord Coord) const noexcept {
    if (Coord.X + ToDim(Unit->GetWidth()) > WIDTH || Coord.Y + ToDim(Unit->GetHeight()) > HEIGHT) {
      return false;
    }
    for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->GetHeight()); Y < EY; ++Y) {
      for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->GetWidth()); X < EX; ++X) {
        if (Units_[Y][X]) {
          return false;
        }
      }
    }
    return true;
  }

  bool TrySetUnit(Unit *Unit, Coord Coord) noexcept {
    if (CanPlaceUnit(Unit, Coord)) {
      SetUnit(Unit, Coord);
      return true;
    }
    return false;
  }

private:
  void SetUnit(Unit *Unit, Coord Coord) noexcept {
    for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->GetHeight()); Y < EY; ++Y) {
      for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->GetWidth()); X < EX; ++X) {
        Units_[Y][X] = Unit;
      }
    }
    Unit->SetPosition(Coord);
  }

  Unit *Units_[HEIGHT][WIDTH];
};

class Player;

class Squad {
public:
  static constexpr Size MAX_LEADERSHIP = 5;

  explicit Squad(Id<Unit> Leader) noexcept : Leader_{Leader} { Units_.push_back(Leader); }

private:
  Grid Grid_;
  SmallVector<Id<Unit>, MAX_LEADERSHIP> Units_;
  Id<Unit> Leader_;
  Id<Player> Player_;
};

} // namespace NotAGame
