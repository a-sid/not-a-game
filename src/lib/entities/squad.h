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
      : Units_{Width * Height, NullId}, Width_{Width}, Height_{Height} {}

  Id<Unit> GetUnit(Coord Coord) const noexcept { return GetUnit(Coord.X, Coord.Y); }
  Id<Unit> GetUnit(Dim X, Dim Y) const noexcept { return Units_[Height_ * Y + X]; }

  Id<Unit> &GetUnit(Coord Coord) noexcept { return GetUnit(Coord.X, Coord.Y); }
  Id<Unit> &GetUnit(Dim X, Dim Y) noexcept { return Units_[Height_ * Y + X]; }

  bool CanPlaceUnit(Id<Unit> UnitId, const Unit *Unit, Coord Coord) const noexcept {
    if (Coord.X + ToDim(Unit->GetWidth()) > Width_ ||
        Coord.Y + ToDim(Unit->GetHeight()) > Height_) {
      return false;
    }
    for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->GetHeight()); Y < EY; ++Y) {
      for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->GetWidth()); X < EX; ++X) {
        if (GetUnit(X, Y).IsValid()) {
          return false;
        }
      }
    }
    return true;
  }

  bool TrySetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept {
    if (CanPlaceUnit(UnitId, Unit, Coord)) {
      SetUnit(UnitId, Unit, Coord);
      return true;
    }
    return false;
  }

private:
  void SetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept {
    for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->GetHeight()); Y < EY; ++Y) {
      for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->GetWidth()); X < EX; ++X) {
        GetUnit(X, Y) = UnitId;
      }
    }
    Unit->SetPosition(Coord);
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
  explicit Squad(const GridSettings &Settings, Utils::Registry<Unit> &Registry,
                 Id<Unit> Leader) noexcept
      : Grid_{Settings.Width, Settings.Height}, Leader_{Leader}, Registry_{Registry} {
    Units_.push_back(Leader);
  }
  Grid &GetGrid() noexcept { return Grid_; }

  Unit *GetUnit(Dim X, Dim Y) const noexcept {
    return Registry_.TryGetObjectById(Grid_.GetUnit(X, Y));
  }

  /*auto GetUnits() const noexcept {
    return Units_ | std::views::transform([this](auto Id) { return Registry_.GetObjectById(Id); });
  }*/

private:
  Grid Grid_;
  SmallVector<Id<Unit>, 8> Units_;
  Id<Unit> Leader_;
  Id<Player> Player_;

  Utils::Registry<Unit> &Registry_;
};

} // namespace NotAGame
