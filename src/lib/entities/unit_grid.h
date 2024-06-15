#pragma once

#include "util/id.h"
#include "util/registry.h"
#include "util/types.h"

#include <cstdint>

namespace NotAGame {

class Unit;

struct GridSettings {
  uint8_t Width;
  uint8_t Height;
};

class Grid {
public:
  Grid(uint8_t Width, uint8_t Height) noexcept
      : Units_(Width * Height, NullId), Width_{Width}, Height_{Height} {}

  Id<Unit> GetUnit(Coord Coord) const noexcept { return GetUnit(Coord.X, Coord.Y); }
  Id<Unit> GetUnit(Dim X, Dim Y) const noexcept { return Units_[Width_ * Y + X]; }

  Id<Unit> &GetUnit(Coord Coord) noexcept { return GetUnit(Coord.X, Coord.Y); }
  Id<Unit> &GetUnit(Dim X, Dim Y) noexcept { return Units_[Width_ * Y + X]; }

  bool CanPlaceUnit(const Unit *Unit, Coord Coord) const noexcept;
  bool TrySetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept;

private:
  void SetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept;

  std::vector<Id<Unit>> Units_;
  uint8_t Width_;
  uint8_t Height_;
};

} // namespace NotAGame
