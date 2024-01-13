#pragma once

#include "entities/unit.h"
#include "util/id.h"
#include "util/registry.h"
#include "util/types.h"

#include <cstdint>
#include <string>

namespace NotAGame {

struct UnitPosition {
  Id<Unit> UnitId;
  Coord Position;
};

class Grid {
public:
  Grid(uint8_t Width, uint8_t Height) noexcept
      : Units_(Width * Height, NullId), Width_{Width}, Height_{Height} {}

  Id<Unit> GetUnit(Coord Coord) const noexcept { return GetUnit(Coord.X, Coord.Y); }
  Id<Unit> GetUnit(Dim X, Dim Y) const noexcept { return Units_[Width_ * Y + X]; }

  Id<Unit> &GetUnit(Coord Coord) noexcept { return GetUnit(Coord.X, Coord.Y); }
  Id<Unit> &GetUnit(Dim X, Dim Y) noexcept { return Units_[Width_ * Y + X]; }

  bool CanPlaceUnit(const Unit *Unit, Coord Coord) const noexcept {
    if (Coord.X + ToDim(Unit->Width) > Width_ || Coord.Y + ToDim(Unit->Height) > Height_) {
      return false;
    }
    for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->Height); Y < EY; ++Y) {
      for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->Width); X < EX; ++X) {
        if (GetUnit(X, Y).IsValid()) {
          return false;
        }
      }
    }
    return true;
  }

  bool TrySetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept {
    if (CanPlaceUnit(Unit, Coord)) {
      SetUnit(UnitId, Unit, Coord);
      return true;
    }
    return false;
  }

private:
  void SetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept {
    for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->Height); Y < EY; ++Y) {
      for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->Width); X < EX; ++X) {
        GetUnit(X, Y) = UnitId;
      }
    }
    Unit->GridPosition = Coord;
  }

  std::vector<Id<Unit>> Units_;
  uint8_t Width_;
  uint8_t Height_;
};

class Player;

struct GridSettings {
  uint8_t Width;
  uint8_t Height;
};

class Squad {
public:
  Squad(const GridSettings &Settings, Id<Unit> Leader, Id<Player> PlayerId) noexcept
      : Grid_{Settings.Width, Settings.Height}, Leader_{Leader}, Player_{PlayerId} {
    Units_.push_back(Leader);
  }
  Grid &GetGrid() noexcept { return Grid_; }

  Id<Unit> GetUnit(Dim X, Dim Y) const noexcept { return Grid_.GetUnit(X, Y); }
  Id<Unit> GetLeader() const noexcept { return Leader_; }

  /*auto GetUnits() const noexcept {
    return Units_ | std::views::transform([this](auto Id) { return Registry_.GetObjectById(Id); });
  }*/

  Id<Squad> ComponentId;

private:
  Grid Grid_;
  SmallVector<Id<Unit>, 8> Units_;
  Id<Unit> Leader_;
  Id<Player> Player_;
};

} // namespace NotAGame
