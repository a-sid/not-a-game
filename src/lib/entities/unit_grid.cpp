#include "entities/unit_grid.h"

#include "entities/unit.h"

namespace NotAGame {

bool Grid::CanPlaceUnit(const Unit *Unit, Coord Coord) const noexcept {
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

bool Grid::TrySetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept {
  if (CanPlaceUnit(Unit, Coord)) {
    SetUnit(UnitId, Unit, Coord);
    return true;
  }
  return false;
}

void Grid::SetUnit(Id<Unit> UnitId, Unit *Unit, Coord Coord) noexcept {
  for (Dim Y = Coord.Y, EY = Coord.Y + ToDim(Unit->Height); Y < EY; ++Y) {
    for (Dim X = Coord.X, EX = Coord.X + ToDim(Unit->Width); X < EX; ++X) {
      GetUnit(X, Y) = UnitId;
    }
  }
  Unit->GridPosition = Coord;
}

} // namespace NotAGame
